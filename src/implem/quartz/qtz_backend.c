/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_QUARTZ

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <mach/mach_time.h>
#include <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>

#include "../hashtable.h"
#include "../event.h"
#include "qtz_keyboard.h"
#include "qtz_window_internal.h"
#include "qtz_backend.h"
#include "qtz_backend_internal.h"

#define ALLOC_POOL NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE_POOL [pool release]

qtz_backend_t *qtz_back = NULL;

static hash_t
_qtz_nswin_hash(
  const NSWindow *nswin)
{
  return (hash_t)((uintptr_t)nswin >> 2);
}

static bool
_qtz_nswin_equal(
  const NSWindow *nswin1,
  const NSWindow *nswin2)
{
  return (nswin1) == (nswin2);
}

int64_t
qtz_get_time(
  void)
{
  assert(qtz_back != NULL);
  uint64_t t = mach_absolute_time();
  /* Process the lower and upper parts separately to maximize precision */
  t =
    ((t & 0xFFFFFFFF00000000) / qtz_back->musec_per_tick_denom) * qtz_back->musec_per_tick_numer +
    ((t & 0x00000000FFFFFFFF) * qtz_back->musec_per_tick_numer) / qtz_back->musec_per_tick_denom;
  return (int64_t)t;
}

static void
_qtz_backend_update_layout(
  void)
{
  TISInputSourceRef inputSourceRef = TISCopyCurrentKeyboardLayoutInputSource();
  CFDataRef layoutDataRef =
    (CFDataRef)TISGetInputSourceProperty(inputSourceRef, kTISPropertyUnicodeKeyLayoutData);
  qtz_back->layout = (UCKeyboardLayout *)CFDataGetBytePtr(layoutDataRef);
  CFRelease(inputSourceRef);
}


bool
qtz_backend_init(
  void)
{
  assert(qtz_back == NULL);

  /* Allocate the backend object */
  qtz_back = (qtz_backend_t *)calloc(1, sizeof(qtz_backend_t));
  if (qtz_back == NULL) {
    return false;
  }

  /* Map from QTZ windows to window objects */
  qtz_back->nswin_to_win = ht_new((key_hash_fun_t *)_qtz_nswin_hash,
                                  (key_equal_fun_t *)_qtz_nswin_equal,
                                  32);
  if (qtz_back->nswin_to_win == NULL) {
    qtz_backend_terminate();
    return false;
  }

  _qtz_backend_update_layout();
  if (qtz_back->layout == NULL) {
    qtz_backend_terminate();
    return false;
  }

  /* Initialize the high performance counter */
  mach_timebase_info_data_t tb_info;
  mach_timebase_info(&tb_info);
  qtz_back->musec_per_tick_numer = tb_info.numer;
  qtz_back->musec_per_tick_denom = tb_info.denom * 1000.0;

  ALLOC_POOL;

  ProcessSerialNumber psn = { 0, kCurrentProcess };
  TransformProcessType(&psn, kProcessTransformToForegroundApplication);

  [NSApplication sharedApplication];

  //  if (![[NSRunningApplication currentApplication] isFinishedLaunching])
  //  [NSApp run];

  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular]; /* needed to have menu + brings app to front */
  [NSApp activateIgnoringOtherApps:YES];

// need autorelease pool ?

  id appName = [[NSProcessInfo processInfo] processName];

  id menubar = [NSMenu new];
  id appMenuItem = [NSMenuItem new];
  [menubar addItem:appMenuItem];
  [NSApp setMainMenu:menubar];

  id appMenu = [NSMenu new];
  id quitTitle = [@"Quit " stringByAppendingString:appName];
  id quitMenuItem = [[NSMenuItem alloc]
      initWithTitle:quitTitle
      action:@selector(terminate:)
      keyEquivalent:@"q"];
  [appMenu addItem:quitMenuItem];

  [appMenuItem setSubmenu:appMenu];

  RELEASE_POOL;

  return true;
}

void
qtz_backend_terminate(
  void)
{
  if (qtz_back == NULL) {
    return;
  }

  if (qtz_back->nswin_to_win != NULL) {
// TODO: iterate to close windows
    ht_delete(qtz_back->nswin_to_win);
  }

  free(qtz_back);

  qtz_back = NULL;
}

void
qtz_backend_add_window(
  qtz_window_t *w)
{
  assert(w != NULL);
  assert(w->nswin != NULL);
  ht_add(qtz_back->nswin_to_win, (void *)w->nswin, (void *)w);
}

void
qtz_backend_remove_window(
  const qtz_window_t *w)
{
  assert(w != NULL);
  assert(w->nswin != NULL);
  ht_remove(qtz_back->nswin_to_win, (void *)w->nswin);
}

qtz_window_t *
qtz_backend_get_window(
  const NSWindow *nswin)
{
  assert(nswin != NULL);
  return (qtz_window_t *)ht_find(qtz_back->nswin_to_win, (void *)nswin);
}

void
qtz_backend_set_listener(
  event_listener_t *listener)
{
  if (listener != NULL) {
    assert(listener->process_event != NULL);
  }
  qtz_back->listener = listener;
}

event_listener_t *
qtz_backend_get_listener(
  void)
{
  return qtz_back->listener;
}

static void
_qtz_render_all_windows(
  void)
{
  event_t evt;
  qtz_window_t *w = NULL;
  hashtable_iterator_t *i = NULL;
  evt.type = EVENT_FRAME;
  evt.time = qtz_get_time();
  i = ht_get_iterator(qtz_back->nswin_to_win);
  if (i != NULL) {
    while ((w = (qtz_window_t *)ht_iterator_next(i)) != NULL) {
      if (w->base.visible == true) {
        evt.target = (void *)w;
        if (event_notify(qtz_back->listener, &evt)) {
          [w->nsview setNeedsDisplay:YES];
        }
      }
    }
    ht_free_iterator(i);
  }
}

static key_code_t
_qtz_key_event_code(
  NSEvent *ev)
{
  switch ([ev type]) {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    case NSEventTypeFlagsChanged:
      return qtz_keyboard_translate_keycode([ev keyCode]);
    default:
      assert(!"Unhandled key event");
  }
  return KEY_UNDEFINED;
}

static int32_t
_qtz_key_event_char(
  NSEvent *ev,
  bool *dead)
{
  switch ([ev type]) {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    case NSEventTypeFlagsChanged:
      return qtz_keyboard_keycode_to_unicode([ev keyCode],
                                             [ev modifierFlags], dead);
    default:
      assert(!"Unhandled key event");
  }
  return 0;
}

static key_modifier_t
_qtz_key_event_modifiers(
  NSEvent *ev)
{
  switch ([ev type]) {
    case NSEventTypeKeyDown:
    case NSEventTypeKeyUp:
    case NSEventTypeFlagsChanged: {
      NSEventModifierFlags modifiers = [ev modifierFlags];
      return
        (modifiers & NSEventModifierFlagShift) ? MOD_SHIFT : 0 |
        (modifiers & NSEventModifierFlagOption) ? MOD_ALT : 0 |
        (modifiers & NSEventModifierFlagControl) ? MOD_CTRL : 0 |
        (modifiers & NSEventModifierFlagCommand) ? MOD_META : 0 |
        (modifiers & NSEventModifierFlagCapsLock) ? MOD_CAPSLOCK : 0;
    }
    default:
      assert(!"Unhandled key event");
  }
  return MOD_NONE;
}

static key_state_t
_qtz_key_event_state(
  NSEvent *ev)
{
  switch ([ev type]) {
    case NSEventTypeKeyDown:
      return KEY_DOWN;
    case NSEventTypeKeyUp:
      return KEY_UP;
    case NSEventTypeFlagsChanged: {
      unsigned short code = [ev keyCode];
      NSEventModifierFlags modifiers = [ev modifierFlags];
      NSEventModifierFlags mask = 0;
      switch (code) {
        case kVK_Shift: mask = NX_DEVICELSHIFTKEYMASK; break;
        case kVK_Control: mask = NX_DEVICELCTLKEYMASK; break;
        case kVK_Option: mask = NX_DEVICELALTKEYMASK; break;
        case kVK_Command: mask = NX_DEVICELCMDKEYMASK; break;
        case kVK_RightShift: mask = NX_DEVICERSHIFTKEYMASK; break;
        case kVK_RightControl: mask = NX_DEVICERCTLKEYMASK; break;
        case kVK_RightOption: mask = NX_DEVICERALTKEYMASK; break;
        case kVK_RightCommand: mask = NX_DEVICERCMDKEYMASK; break;
        case kVK_CapsLock: mask = NSEventModifierFlagCapsLock; break;
        // NX_DEVICE_ALPHASHIFT_STATELESS_MASK
        // kVK_Function ?
        default: mask = 0; break;
      }
      key_state_t state = (modifiers & mask) ? KEY_DOWN : KEY_UP;
      return state;
    }
    default:
      assert(!"Unhandled key event state");
  }
  return KEY_UP;
}

static button_t
_qtz_mouse_event_button(
  NSEvent *ev)
{
  switch ([ev type]) {
    case NSEventTypeLeftMouseDown:
    case NSEventTypeLeftMouseUp:
      return BUTTON_LEFT;
    case NSEventTypeRightMouseDown:
    case NSEventTypeRightMouseUp:
      return BUTTON_RIGHT;
    case NSEventTypeOtherMouseDown:
    case NSEventTypeOtherMouseUp:
      return BUTTON_MIDDLE;
    case NSEventTypeScrollWheel:
      if (ev.deltaZ > 0.0) { return BUTTON_WHEEL_UP; }
      else { return BUTTON_WHEEL_DOWN; }
    default:
      assert(!"Unhandled mouse event button");
  }
  return BUTTON_NONE;
}

static button_state_t
_qtz_mouse_event_state(
  NSEvent *ev)
{
  switch ([ev type]) {
    case NSEventTypeLeftMouseDown:
    case NSEventTypeRightMouseDown:
    case NSEventTypeOtherMouseDown:
    case NSEventTypeScrollWheel:
      return BUTTON_DOWN;
    case NSEventTypeLeftMouseUp:
    case NSEventTypeRightMouseUp:
    case NSEventTypeOtherMouseUp:
      return BUTTON_UP;
    default:
      assert(!"Unhandled mouse event state");
  }
  return BUTTON_UP;
}

static qtz_window_t *
_qtz_window_at_point(
  int32_t x,
  int32_t y)
{
  NSPoint p = NSMakePoint(x, y);
  NSInteger wn = [NSWindow windowNumberAtPoint:p belowWindowWithWindowNumber:0];
  NSWindow *nswin = [NSApp windowWithWindowNumber:wn];
  if (nswin == NULL) {
    return NULL;
  } else {
    return qtz_backend_get_window(nswin);
  }
}

static bool
_qtz_point_in_window_area(
  qtz_window_t *w,
  int32_t x,
  int32_t y)
{
  if ((x < 5) || (x > w->base.width - 5) ||
      (y < 5) || (y > w->base.height - 5) ||
      ((x < 15 ) && (y < 15)) ||
      ((x > w->base.width - 15) && (y < 15))) {
    return false;
  }
  return true;
}

void
qtz_backend_run(
  void)
{
  assert(qtz_back != NULL);
  assert(qtz_back->running == false);

  qtz_window_t *w = NULL;
  NSEvent *ev = NULL;
  event_t evt;

  qtz_back->running = true;

  CFRunLoopRef runloop = CFRunLoopGetMain();
  CFRetain(runloop);

  CFRunLoopTimerRef runloopTimer = CFRunLoopTimerCreateWithHandler(
    kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + 0.0166667, 0.0166667, 0, 0,
    ^(CFRunLoopTimerRef timer) { _qtz_render_all_windows(); });

  CFRunLoopAddTimer(runloop, runloopTimer, kCFRunLoopCommonModes);
  //CFRunLoopAddTimer(runloop, runloopTimer, NSEventTrackingRunLoopMode);

  while (qtz_back->running) {

    ALLOC_POOL;

    ev = [NSApp nextEventMatchingMask:NSEventMaskAny // before 10.12: NSAnyEventMask
      untilDate:NSDate.distantFuture inMode:NSDefaultRunLoopMode dequeue:YES];

    if (ev) {

      [NSApp sendEvent:ev];
      if ([ev window] == NULL) {
        [ev release];
        continue;
      }

      switch ([ev type]) {

        case NSEventTypeKeyDown:
        case NSEventTypeKeyUp:
          if ([ev isARepeat]) {
            // how to deal with repeat ? Send multiple events ?
            // does X11/win32 allow to dinstinguish repeats ?
            break;
          }
          /* fallthrough */
        case NSEventTypeFlagsChanged:
          w = qtz_backend_get_window([ev window]);
          if (w != NULL) {
            evt.type = EVENT_KEY;
            evt.time = qtz_get_time();
            evt.target = (void *)w;
            evt.desc.key.code = _qtz_key_event_code(ev);
            evt.desc.key.char_ = _qtz_key_event_char(ev, &(evt.desc.key.dead));
            evt.desc.key.modifiers = _qtz_key_event_modifiers(ev);
            evt.desc.key.state = _qtz_key_event_state(ev);
            //[NSApp sendEvent:ev];
            event_notify(qtz_back->listener, &evt);
          }
          break;

        case NSEventTypeLeftMouseDown:
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseDown:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseDown:
        case NSEventTypeOtherMouseUp:
        case NSEventTypeScrollWheel:
          w = qtz_backend_get_window([ev window]);
          if (w != NULL) {
            int32_t x = (ev.locationInWindow.x);
            int32_t y = (ev.locationInWindow.y); // -1 ?
            if (!_qtz_point_in_window_area(w, x, y)) {
              //[NSApp sendEvent:ev];
              break;
            }
            y = w->base.height - y;
            evt.type = EVENT_BUTTON;
            evt.time = qtz_get_time();
            evt.target = (void *)w;
            evt.desc.button.x = x;
            evt.desc.button.y = y;
            evt.desc.button.button = _qtz_mouse_event_button(ev);
            evt.desc.button.state = _qtz_mouse_event_state(ev);
            //[NSApp sendEvent:ev];
            event_notify(qtz_back->listener, &evt);
          }
          break;

          /*
                  case NSEventTypeMouseEntered:
                    printf("Mouse entered\n");
                    break;
                  case NSEventTypeMouseExited:
                    printf("Mouse exited\n");
                    break;
          */
        case NSEventTypeMouseMoved:
        case NSEventTypeLeftMouseDragged:
        case NSEventTypeRightMouseDragged:
        case NSEventTypeOtherMouseDragged: {
          int32_t x = ev.locationInWindow.x;
          int32_t y = ev.locationInWindow.y; // -1 ?
          if ([ev window] != NULL) {
            w = qtz_backend_get_window([ev window]);
          } else {
            w = _qtz_window_at_point(x, y);
            if (w != NULL) {
              NSRect r = NSMakeRect(x, y, 0, 0);
              r = [w->nswin convertRectFromScreen:r];
              x = r.origin.x;
              y = r.origin.y;
            }
          }
          if ((w != NULL) && (_qtz_point_in_window_area(w, x, y))) {
            y = w->base.height - y;
            evt.type = EVENT_CURSOR;
            evt.time = qtz_get_time();
            evt.target = (void *)w;
            evt.desc.cursor.x = x;
            evt.desc.cursor.y = y;
            //[NSApp sendEvent:ev];
            event_notify(qtz_back->listener, &evt);
          } else {
            ///[NSApp sendEvent:ev];
          }
          break;
        }

        default:
        //printf("Unknown event (%d, %d)\n", [ev type], [ev subtype]);
          //[NSApp sendEvent:ev];
          break;
      }
      //[ev release];
    }

    RELEASE_POOL;
  }

  CFRunLoopRemoveTimer(runloop, runloopTimer, kCFRunLoopCommonModes);
  CFRelease(runloopTimer);
  CFRelease(runloop);
}

void
qtz_backend_stop(
  void)
{
  assert(qtz_back != NULL);

  qtz_back->running = false;
}

#else

const int qtz_backend = 0;

#endif /* HAS_QUARTZ */

/*
CFRunLoopObserverRef runloopObserver = CFRunLoopObserverCreateWithHandler(
  kCFAllocatorDefault, kCFRunLoopAllActivities, YES, 0,
  ^(CFRunLoopObserverRef observer, CFRunLoopActivity activity) {
// 1 2 4 128
// 1 2 4 32,64 128
// 1 = Entry
// 2 = BeforeTimers
// 4 = BeforeSources
// 32 = BeforeWaiting
// 64 = AfterWaiting
// 128 = RunLoopExit
  });

CFRunLoopAddObserver(runloop, runloopObserver, kCFRunLoopDefaultMode);
//CFRunLoopAddObserver(runloop, runloopObserver, NSEventTrackingRunLoopMode);
*/
/*
CFRunLoopTimerRef runloopTimer = CFRunLoopTimerCreateWithHandler(
  kCFAllocatorDefault, CFAbsoluteTimeGetCurrent() + 0.01667, 0.01667, 0, 0,
  ^(CFRunLoopTimerRef timer) {
    _qtz_render_all_windows();
  });

//CFRunLoopAddTimer(runloop, runloopTimer, kCFRunLoopDefaultMode);
CFRunLoopAddTimer(runloop, runloopTimer, kCFRunLoopCommonModes);
//CFRunLoopAddTimer(runloop, runloopTimer, NSEventTrackingRunLoopMode);
//[NSThread detachNewThreadSelector:@selector(aMethod:) toTarget:[MyObject class] withObject:nil];
*/

/*

  while (qtz_back->running) {
    ev = [NSApp nextEventMatchingMask:NSEventMaskAny // before 10.12: NSAnyEventMask
      untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
      // distantFuture
    if (ev) {
/ *
      if (([ev type] == NSSystemDefined))
        printf("Event: %ld (%d)\n", [ev type], [ev subtype]);
      else
        printf("Event: %ld\n", [ev type]);

        if ([ev type] == NSEventTypeLeftMouseDown) {
          printf("LMD, user=%d, prog=%d\n", user_lmd, prog_lme);
          if (!prog_lme)
             user_lmd = true;
        } else if ([ev type] == NSEventTypeLeftMouseUp) {
          printf("LMU, user=%d, prog=%d\n", user_lmd, prog_lme);
          if (!prog_lme)
            user_lmd = false;
          else
            prog_lme = false;
        };
*/
        /*
      if ([ev type] == NSEventTypeLeftMouseDown) {
        NSWindow *w = [ev window];
        NSRect frame = [w frame];
        printf("Clicked window (x,y), (w, h) = (%f, %f), (%f, %f)\n", frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        ev locationInWindow
      } else {*//*
        [NSApp sendEvent:ev];
      //}
    } else {
      // Wait
      [NSThread sleepForTimeInterval:0.008];
    }
//printf("%d\n", xxx++);
    // frame if needed
    //cur_time = _gdi_get_time();

//    if (1
          / *cur_time - gdi_back->last_frame >= 1000000/60*//*) {
      //gdi_back->last_frame = cur_time;
//      _qtz_render_all_windows();
//    }

//  };
*/
