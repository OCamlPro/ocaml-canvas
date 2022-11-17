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
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <unistd.h>

#include <Cocoa/Cocoa.h>

#include "../util.h"
#include "qtz_backend.h"
#include "qtz_backend_internal.h"
#include "qtz_target.h"
#include "qtz_window_internal.h"

#define ALLOC_POOL NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE_POOL [pool release]

@interface CanvasWindow : NSWindow <NSDraggingDestination>

@end

@implementation CanvasWindow

- (BOOL)canBecomeMainWindow
{
  return YES;
}

- (BOOL)canBecomeKeyWindow
{
  return YES;
}

- (BOOL)preservesContentDuringLiveResize
{
  return TRUE;
}

- (instancetype)initWithContentRect:(NSRect)contentRect
                          styleMask:(NSWindowStyleMask)style
                            backing:(NSBackingStoreType)backingStoreType
                              defer:(BOOL)flag
{
  [[NSNotificationCenter defaultCenter]
    addObserver:self
      selector:@selector(windowDidBecomeKey:)
          name:NSWindowDidBecomeKeyNotification
        object:self];
  [[NSNotificationCenter defaultCenter]
    addObserver:self
       selector:@selector(windowDidResignKey:)
           name:NSWindowDidResignKeyNotification
        object:self];
  [[NSNotificationCenter defaultCenter]
    addObserver:self
       selector:@selector(windowDidMove:)
           name:NSWindowDidMoveNotification
        object:self];
  return [super initWithContentRect:contentRect
                          styleMask:style
                            backing:backingStoreType
                              defer:flag];
}

//- (void)windowWillClose:(NSNotification *)notification
- (BOOL)windowShouldClose:(NSWindow *)sender
{
  qtz_window_t *w = qtz_backend_get_window(self);
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_CLOSE;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    event_notify(qtz_back->listener, &evt);
  }
  return NO;
}

// window is being resized by user or setFrame
//- (void)windowDidResize:(NSNotification *)notification
- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)frameSize
{
  qtz_window_t *w = qtz_backend_get_window(self);
  if (w != NULL) {
    event_t evt;
    NSRect frect = NSMakeRect(0, 0, frameSize.width, frameSize.height);
    NSRect crect = [self contentRectForFrameRect:frect];
    w->base.width = crect.size.width;
    w->base.height = crect.size.height;
    evt.type = EVENT_RESIZE;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    evt.desc.resize.width = w->base.width;
    evt.desc.resize.height = w->base.height;
    event_notify(qtz_back->listener, &evt);
  }
  return frameSize;
}

- (void)windowDidMove:(NSNotification *)notification
{
  qtz_window_t *w = qtz_backend_get_window(self);
  if (w != NULL) {
    event_t evt;
    NSRect frect = [self frame];
    NSRect crect = [self contentRectForFrameRect:frect];
    NSRect srect = [[NSScreen mainScreen] frame];
    w->base.x = crect.origin.x;
    w->base.y = srect.size.height - crect.origin.y -crect.size.height;
    evt.type = EVENT_MOVE;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    evt.desc.move.x = w->base.x;
    evt.desc.move.y = w->base.y;
    event_notify(qtz_back->listener, &evt);
  }
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
  qtz_window_t *w = qtz_backend_get_window(self);
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_FOCUS;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    evt.desc.focus.inout = FOCUS_IN;
    event_notify(qtz_back->listener, &evt);
  }
}

- (void)windowDidResignKey:(NSNotification *)notification
{
  qtz_window_t *w = qtz_backend_get_window(self);
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_FOCUS;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    evt.desc.focus.inout = FOCUS_OUT;
    event_notify(qtz_back->listener, &evt);
  }
}

@end

@interface CanvasView : NSView

@end

@implementation CanvasView { }

- (BOOL)isFlipped
{
  return FALSE;
}

- (BOOL)preservesContentDuringLiveResize
{
  return TRUE;
}

- (void)drawRect:(NSRect)rect
{
  qtz_window_t *w = qtz_backend_get_window([self window]);
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_PRESENT;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    event_notify(qtz_back->listener, &evt);
  }
}

@end



static void
_qtz_window_update_size_and_position(
  qtz_window_t *window)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  ALLOC_POOL;
  NSRect srect = [[NSScreen mainScreen] frame];
  NSRect crect = NSMakeRect(window->base.x,
                            srect.size.height - window->base.y
                                              - window->base.height,
                            window->base.width, window->base.height);
  NSRect frect = [window->nswin frameRectForContentRect:crect];
  [window->nswin setFrame:frect display:TRUE];
  RELEASE_POOL;
}

qtz_window_t *
qtz_window_create(
  bool decorated,
  bool resizeable,
  bool minimize,
  bool maximize,
  bool close,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  qtz_window_t *window = (qtz_window_t *)calloc(1, sizeof(qtz_window_t));
  if (window == NULL) {
    return NULL;
  }

  window->base.visible = false;
  window->base.decorated = decorated;
  window->base.resizeable = resizeable;
  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  window->base.width = clip_i32_to_i16(max(1, width));
  window->base.height = clip_i32_to_i16(max(1, height));

  ALLOC_POOL;

  NSRect crect = NSMakeRect(window->base.x, window->base.y,
                            window->base.width, window->base.height);

  NSWindowStyleMask mask;
  if (window->base.decorated) {
    mask = NSWindowStyleMaskTitled |
      (resizeable ? NSWindowStyleMaskResizable : 0) |
      (minimize ? NSWindowStyleMaskMiniaturizable : 0) |
      (close ? NSWindowStyleMaskClosable : 0);
  } else {
    mask = NSWindowStyleMaskBorderless;
  }

  window->nswin = [[CanvasWindow alloc]
    initWithContentRect:crect styleMask:mask
    backing:NSBackingStoreBuffered defer:NO];

  NSRect frect = [window->nswin frameRectForContentRect:crect];
  window->nsview = [[CanvasView alloc] initWithFrame:frect];
  [window->nsview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [window->nswin setContentView:window->nsview];

  if (maximize == false) {
    NSButton *b = [window->nswin standardWindowButton:NSWindowZoomButton];
    [b setEnabled:NO];
  }

  RELEASE_POOL;

  _qtz_window_update_size_and_position(window);

  /* Set the title */
  const char *t = (title != NULL) ? title : "";
  qtz_window_set_title(window, t);

  /* Add to managed winddows */
  qtz_backend_add_window(window);

  return window;
}

void
qtz_window_destroy(
  qtz_window_t *window)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  qtz_backend_remove_window(window);
  ALLOC_POOL;
  [window->nswin close];
  RELEASE_POOL;
  free(window);
}

qtz_target_t *
qtz_window_get_target(
  qtz_window_t *window)
{
  assert(window != NULL);
  assert(window->nsview != NULL);

  return qtz_target_create(window->nsview);
}

void
qtz_window_set_title(
  qtz_window_t *window,
  const char *title)
{
  assert(window != NULL);
  assert(window->nswin != NULL);
  assert(title != NULL);

  ALLOC_POOL;
  [window->nswin setTitle:[NSString stringWithUTF8String:title]];
  RELEASE_POOL;
}

void
qtz_window_set_size(
  qtz_window_t *window,
  int32_t width,
  int32_t height)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  window->base.width = clip_i32_to_i16(width);
  window->base.height = clip_i32_to_i16(height);
  _qtz_window_update_size_and_position(window);
}

void
qtz_window_set_position(
  qtz_window_t *window,
  int32_t x,
  int32_t y)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  window->base.x = clip_i32_to_i16(x);
  window->base.y = clip_i32_to_i16(y);
  _qtz_window_update_size_and_position(window);
}

void
qtz_window_show(
  qtz_window_t *window)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  ALLOC_POOL;
  [window->nswin makeKeyAndOrderFront:window->nswin];
  //[window->nswin orderFront:nil];
  RELEASE_POOL;
}

void
qtz_window_hide(
  qtz_window_t *window)
{
  assert(window != NULL);
  assert(window->nswin != NULL);

  ALLOC_POOL;
  [window->nswin orderOut:nil];
  RELEASE_POOL;
}

#else

const int qtz_window = 0;

#endif /* HAS_QUARTZ */
