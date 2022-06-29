/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_GDI

#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "../hashtable.h"
#include "../event.h"
#include "gdi_keyboard.h"
#include "gdi_window_internal.h"
#include "gdi_backend_internal.h"

gdi_backend_t *gdi_back = NULL;

static hash_t
_gdi_hwnd_hash(
  const HWND *hwnd)
{
  return (hash_t)(uintptr_t)*hwnd;
}

static bool
_gdi_hwnd_equal(
  const HWND *hwnd1,
  const HWND *hwnd2)
{
  return (*hwnd1) == (*hwnd2);
}

int64_t
gdi_get_time(
  void)
{
  assert(gdi_back != NULL);

  LARGE_INTEGER t;
  if (!QueryPerformanceCounter(&t)) {
    /* Should never happen on Windows XP and above */
    return 0;
  }
  return (int64_t)((double)t.QuadPart * gdi_back->musec_per_tick);
}

static LRESULT CALLBACK
_gdi_window_proc(
  HWND hwnd,
  UINT msg,
  WPARAM wparam,
  LPARAM lparam);

bool
gdi_backend_init(
  void)
{
  assert(gdi_back == NULL);

  /* Allocate the backend object */
  gdi_back = (gdi_backend_t *)calloc(1, sizeof(gdi_backend_t));
  if (gdi_back == NULL) {
    return false;
  }

  /* Map from Win32 windows IDs to window objects */
  gdi_back->hwnd_to_win = ht_new((key_hash_fun_t *)_gdi_hwnd_hash,
                                 (key_equal_fun_t *)_gdi_hwnd_equal,
                                 32);
  if (gdi_back->hwnd_to_win == NULL) {
    free(gdi_back);
    return false;
  }

  /* Disable SEH for timer procedures (security recommendation) */
  if (SetUserObjectInformation(GetCurrentProcess(),
                               UOI_TIMERPROC_EXCEPTION_SUPPRESSION,
                               (PVOID)(BOOL[1]){ FALSE },
                               sizeof(BOOL)) == FALSE) {
    ht_delete(gdi_back->hwnd_to_win);
    free(gdi_back);
    return false;
  }

  /* Initialize the high performance counter */
  LARGE_INTEGER f;
  if (!QueryPerformanceFrequency(&f) || f.QuadPart == 0) {
    /* Should never happen on Windows XP and above */
    ht_delete(gdi_back->hwnd_to_win);
    free(gdi_back);
    return false;
  }
  gdi_back->musec_per_tick = 1000000.0 / (double)f.QuadPart;

  gdi_back->hinst = GetModuleHandle(NULL);
  gdi_back->class_framed = L"FRAMED";
  gdi_back->class_frameless = L"FRAMELESS";

  WNDCLASSEXW wcex;
  wcex.cbSize         = sizeof(WNDCLASSEX);
  wcex.style          = 0; //CS_HREDRAW | CS_VREDRAW; // cause flicker
  wcex.cbClsExtra     = 0;
  wcex.cbWndExtra     = 0;
  wcex.hIcon          = NULL;
  wcex.hIconSm        = NULL;
  wcex.lpszMenuName   = NULL;
  wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground  = NULL; //(HBRUSH)(COLOR_WINDOW+1);
  wcex.hInstance      = gdi_back->hinst;
  wcex.lpfnWndProc    = _gdi_window_proc;

  wcex.lpszClassName  = gdi_back->class_framed;
  RegisterClassExW(&wcex);

  wcex.lpszClassName  = gdi_back->class_frameless;
  RegisterClassExW(&wcex);

  return true;
}

void
gdi_backend_terminate(
  void)
{
  if (gdi_back == NULL) {
    return;
  }

  if (gdi_back->hwnd_to_win != NULL) {
// delete windows
    ht_delete(gdi_back->hwnd_to_win);
  }

  free(gdi_back);

  gdi_back = NULL;
}

void
gdi_backend_add_window(
  gdi_window_t *w)
{
  assert(gdi_back != NULL);
  assert(w != NULL);
  assert(w->hwnd != NULL);

  ht_add(gdi_back->hwnd_to_win, (void *)&(w->hwnd), (void *)w);
}

void
gdi_backend_remove_window(
  const gdi_window_t *w)
{
  assert(gdi_back != NULL);
  assert(w != NULL);
  assert(w->hwnd != NULL);

  ht_remove(gdi_back->hwnd_to_win, (void *)&(w->hwnd));
}

gdi_window_t *
gdi_backend_get_window(
  HWND hwnd)
{
  assert(gdi_back != NULL);
  assert(hwnd != NULL);

  return (gdi_window_t *)ht_find(gdi_back->hwnd_to_win, (void *)&hwnd);
}

void
gdi_backend_set_listener(
  event_listener_t *listener)
{
  assert(gdi_back != NULL);

  if (listener != NULL) {
    assert(listener->process_event != NULL);
  }
  gdi_back->listener = listener;
}

event_listener_t *
gdi_backend_get_listener(
  void)
{
  assert(gdi_back != NULL);

  return gdi_back->listener;
}

static void
_gdi_present_window(
  gdi_window_t *w) // reac to WM_PAINT ?
{
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_PRESENT;
    evt.time = gdi_get_time();
    evt.target = (void *)w;
    evt.desc.present.data.gdi.use_begin = false;//with user data field
    event_notify(gdi_back->listener, &evt);
  }
}

static void
_gdi_render_all_windows(
  void)
{
  event_t evt;
  gdi_window_t *w = NULL;
  hashtable_iterator_t *i = NULL;
  evt.type = EVENT_FRAME;
  evt.time = gdi_get_time();
  i = ht_get_iterator(gdi_back->hwnd_to_win);
  if (i != NULL) {
    while ((w = (gdi_window_t *)ht_iterator_next(i)) != NULL) {
      if (w->base.visible == true) {
        evt.target = (void *)w;
        if (event_notify(gdi_back->listener, &evt)) {
          _gdi_present_window(w);
        }
      }
    }
    ht_free_iterator(i);
  }
}

static key_modifier_t
_gdi_get_keyboard_modifiers(
  void)
{
  return
    (GetKeyState(VK_SHIFT) & 0x80000000) ? MOD_SHIFT : 0 |
    (GetKeyState(VK_MENU) & 0x80000000) ? MOD_ALT : 0 |
    (GetKeyState(VK_CONTROL) & 0x80000000) ? MOD_CTRL : 0 |
    (GetKeyState(VK_LWIN) & 0x80000000) ? MOD_META : 0 |
    (GetKeyState(VK_RWIN) & 0x80000000) ? MOD_META : 0 |
    (GetKeyState(VK_CAPITAL) & 0x00000001) ? MOD_CAPSLOCK : 0;
}

static button_t
_gdi_mouse_event_button(
  UINT msg,
  WPARAM wparam,
  LPARAM lparam)
{
  switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      return BUTTON_LEFT;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return BUTTON_RIGHT;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      return BUTTON_MIDDLE;
    case WM_MOUSEWHEEL:
      if ((wparam & 0xFFFF0000) > 0) { return BUTTON_WHEEL_UP; }
      else { return BUTTON_WHEEL_DOWN; }
    default:
      assert(!"Unhandled mouse event button");
  }
  return (button_t)0;
}

static button_state_t
_gdi_mouse_event_state(
  UINT msg,
  WPARAM wparam,
  LPARAM lparam)
{
  switch (msg) {
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_MOUSEWHEEL:
      return BUTTON_DOWN;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
      return BUTTON_UP;
    default:
      assert(!"Unhandled mouse event state");
  }
  return (button_state_t)0;
}

void
gdi_backend_run(
  void)
{
  assert(gdi_back != NULL);
  assert(gdi_back->running == false);

  gdi_back->running = true;
  gdi_back->next_frame = gdi_get_time();

  while (gdi_back->running) {
    MSG msg;
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        gdi_back->running = false;
      } else {
        TranslateMessage(&msg); // translates WM_KEY* messages to WM_CHAR
        DispatchMessage(&msg);
      }
    } else {
      int64_t cur_time = gdi_get_time();
      int64_t timeout = (gdi_back->next_frame - cur_time) / 1000;
      if ((timeout < 0) ||
          (WAIT_TIMEOUT == MsgWaitForMultipleObjects(0, NULL, TRUE, timeout,
                                                     QS_ALLEVENTS))) {
        _gdi_render_all_windows();
        do {
          gdi_back->next_frame += 1000000 / 60;
        } while (gdi_back->next_frame < cur_time);
      }
    }
  }
}

void
gdi_backend_stop(
  void)
{
  assert(gdi_back != NULL);

  gdi_back->running = false;
}

static VOID CALLBACK
_gdi_modal_timer_proc(
  HWND hwnd,
  UINT msg,
  UINT_PTR id,
  DWORD time)
{
  int64_t cur_time = gdi_get_time();
  int64_t timeout = (gdi_back->next_frame - cur_time) / 1000;
  if (timeout <= 4) {
    _gdi_render_all_windows();
    do {
      gdi_back->next_frame += 1000000 / 60;
    } while (gdi_back->next_frame < cur_time);
  }
}

static LRESULT CALLBACK
_gdi_window_proc(
  HWND hwnd,
  UINT msg,
  WPARAM wparam,
  LPARAM lparam)
{
  gdi_window_t *w = NULL;
  event_t evt;

  if (gdi_back->listener == NULL) {
    printf("Listener not set for event %d\n", msg);
/*
   1     WM_CREATE
   3     WM_MOVE
   5     WM_SIZE
   6     WM_ACTIVATE
   7     WM_SETFOCUS
  20(x2) WM_ERASEBKGND
  24     WM_SHOWWINDOW
  28     WM_ACTIVATEAPP
  36     WM_GETMINMAXINFO
  70(x3) WM_WINDOWPOSCHANGING
  71     WM_WINDOWPOSCHANGED
 127(x3) WM_GETICON
 129     WM_NCCREATE
 131(x2) WM_NCCALCSIZE
 133(x2) WM_NCPAINT
 134     WM_NCACTIVATE
 641     WM_IME_SETCONTEXT
 642     WM_IME_NOTIFY
*/
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  switch (msg) {

    case WM_ENTERIDLE:
    case WM_ENTERSIZEMOVE:
      if (!gdi_back->modal_op) {
        gdi_back->modal_op = true;
        gdi_back->modal_timer = SetTimer(NULL, 0, 1, _gdi_modal_timer_proc);
        if (gdi_back->modal_timer == 0)
          printf("Set timer failed\n");
      }
      return 0;

    case WM_CAPTURECHANGED:
    case WM_EXITSIZEMOVE:
      if (gdi_back->modal_op) {
        gdi_back->modal_op = false;
        KillTimer(NULL, gdi_back->modal_timer);
      }
      return 0;

    case WM_ERASEBKGND:
      return 1; /* Must return non-zero to indicate message processed */

    case WM_PAINT: {
      PAINTSTRUCT ps;
      /*HDC hdc =*/ BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
/*
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        bool paint = true;
        evt.type = EVENT_EXPOSE;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.expose.data = (void *)&paint;
        event_notify(gdi_back->listener, &evt);
      }
*/
      return 0;
    }

/*
    // Window size/position changed by SetWindowPos or window management fct
    // DefWindowProc generates WM_SIZE/WM_MOVE messages
    case WM_WINDOWPOSCHANGED:
      w = gdi_backend_get_window(msg.hwnd);
      if (w != NULL) {
        WINDOWPOS *window_pos = (WINDOWPOS *)(msg.lParam);
        int width = window_pos->cx;
        int height = window_pos->cy;
        //int width = (msg.lParam & 0x0000FFFF);
        //int height = (msg.lParam & 0xFFFF0000) >> 16;
        evt.type = EVENT_RESIZE;
        evt.target = (void *)w;
        evt.desc.resize.width = width;
        evt.desc.resize.height = height;
        event_notify(listener, &evt);
        }
      return 0;
*/

    case WM_DESTROY:
   //   PostQuitMessage(0);
  // send special message when all windows closed ?
      return 0;

    case WM_CLOSE: // close button clicked
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        evt.type = EVENT_CLOSE;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_SIZE:
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        w->base.width = (lparam & 0x0000FFFF);
        w->base.height = (lparam & 0xFFFF0000) >> 16;
        evt.type = EVENT_RESIZE;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.resize.width = w->base.width;
        evt.desc.resize.height = w->base.height;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_MOVE:
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        w->base.x = (lparam & 0x0000FFFF);
        w->base.y = (lparam & 0xFFFF0000) >> 16;
        evt.type = EVENT_MOVE;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.move.x = w->base.x;
        evt.desc.move.y = w->base.y;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_SETFOCUS:
    case WM_KILLFOCUS:
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        evt.type = EVENT_FOCUS;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.focus.inout =
          (msg == WM_SETFOCUS) ? FOCUS_IN : FOCUS_OUT;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_APPCOMMAND:
      printf("App command: %02llX\n", wparam);
      fflush(stdout);
      return 0;

    /* Note: for some reason, there are two messages when CapsLock is pressed */
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
      /* Ignore repeats */
      if ((lparam & 0xC0000000) == 0x40000000) {
        return 0;
      }
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        uint16_t scancode = (lparam & 0x00FF0000) >> 16;
        bool extended = (lparam & 0x01000000) != 0;
        evt.type = EVENT_KEY;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.key.code =
          gdi_keyboard_translate_scancode(scancode, extended);
        evt.desc.key.char_ =
          gdi_keyboard_scancode_to_unicode(wparam, scancode,
                                           &evt.desc.key.dead);
        evt.desc.key.modifiers = _gdi_get_keyboard_modifiers();
        evt.desc.key.state =
          (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) ? KEY_DOWN : KEY_UP;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEWHEEL:
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        evt.type = EVENT_BUTTON;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.button.x = (lparam & 0x0000FFFF);
        evt.desc.button.y = (lparam & 0xFFFF0000) >> 16;
        evt.desc.button.button = _gdi_mouse_event_button(msg, wparam, lparam);
        evt.desc.button.state = _gdi_mouse_event_state(msg, wparam, lparam);
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

    case WM_MOUSEMOVE:
      w = gdi_backend_get_window(hwnd);
      if (w != NULL) {
        evt.type = EVENT_CURSOR;
        evt.time = gdi_get_time();
        evt.target = (void *)w;
        evt.desc.cursor.x = (lparam & 0x0000FFFF);
        evt.desc.cursor.y = (lparam & 0xFFFF0000) >> 16;
        event_notify(gdi_back->listener, &evt);
      }
      return 0;

 // WM_MOUSEACTIVATE

    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  assert(!"Unhandled event");
}

#else

const int gdi_backend = 0;

#endif /* HAS_GDI */
