/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_X11

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <sys/select.h>

#include <xcb/xcb.h>
#include <xcb/shm.h>
#include <xcb/xkb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_keysyms.h>

#include "../util.h"
#include "../hashtable.h"
#include "../event.h"
#include "x11_keysym.h"
#include "x11_keyboard.h"
#include "x11_backend.h"
#include "x11_backend_internal.h"
#include "x11_window_internal.h"

typedef struct xcb_xkb_any_event_t {
  uint8_t         response_type;
  uint8_t         xkbType;
  uint16_t        sequence;
  xcb_timestamp_t time;
  uint8_t         deviceID;
} xcb_xkb_any_event_t;

typedef union xcb_event_t {
  xcb_generic_event_t *generic;
  xcb_key_press_event_t *key_press;
  xcb_key_release_event_t *key_release;
  xcb_button_press_event_t *button_press;
  xcb_button_release_event_t *button_release;
  xcb_enter_notify_event_t *enter_notify;
  xcb_leave_notify_event_t *leave_notify;
  xcb_motion_notify_event_t *motion_notify;
  xcb_keymap_notify_event_t *keymap_notify;
  xcb_expose_event_t *expose;
  xcb_graphics_exposure_event_t *graphics_exposure;
  xcb_no_exposure_event_t *no_exposure;
  xcb_visibility_notify_event_t *visibility_notify;
  xcb_configure_notify_event_t *configure_notify;
  xcb_circulate_notify_event_t *circulate_notify;
  xcb_create_notify_event_t *create_notify;
  xcb_destroy_notify_event_t *destroy_notify;
  xcb_unmap_notify_event_t *unmap_notify;
  xcb_map_notify_event_t *map_notify;
  xcb_reparent_notify_event_t *reparent_notify;
  xcb_gravity_notify_event_t *gravity_notify;
  xcb_resize_request_event_t *resize_request;
  xcb_map_request_event_t *map_request;
  xcb_configure_request_event_t *configure_request;
  xcb_circulate_request_event_t *circulate_request;
  xcb_focus_in_event_t *focus_in;
  xcb_focus_out_event_t *focus_out;
  xcb_property_notify_event_t *property_notify;
  xcb_colormap_notify_event_t *colormap_notify;
  xcb_selection_clear_event_t *selection_clear;
  xcb_selection_request_event_t *selection_request;
  xcb_selection_notify_event_t *selection_notify;
  xcb_client_message_event_t *client_message;
  xcb_mapping_notify_event_t *mapping_notify;
  xcb_ge_generic_event_t *ge_generic;
  xcb_shm_completion_event_t *shm_completion;
  xcb_xkb_any_event_t *xkb_any;
  xcb_xkb_new_keyboard_notify_event_t *xkb_new_keyboard_notify;
  xcb_xkb_map_notify_event_t *xkb_map_notify;
  xcb_xkb_state_notify_event_t *xkb_state_notify;
} xcb_event_t;

x11_backend_t *x11_back = NULL;

static hash_t
_x11_wid_hash(
  const xcb_window_t *wid)
{
  return (hash_t)(uintptr_t)*wid;
}

static bool
_x11_wid_equal(
  const xcb_window_t *wid1,
  const xcb_window_t *wid2)
{
  return (*wid1) == (*wid2);
}

static xcb_atom_t
_x11_query_atom(
  xcb_connection_t *c,
  const char *atom_name)
{
  assert(c != NULL);
  assert(atom_name != NULL);

  xcb_intern_atom_cookie_t cookie =
    xcb_intern_atom(c, 0, strlen(atom_name), atom_name);
  xcb_intern_atom_reply_t *atom_reply =
    xcb_intern_atom_reply(c, cookie, NULL);

  xcb_atom_t atom = XCB_ATOM_NONE;
  if (atom_reply) {
    atom = atom_reply->atom;
    free(atom_reply);
  }

  return atom;
}

int64_t
x11_get_time(
  void)
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

bool
x11_backend_init(
  void)
{
  assert(x11_back == NULL);

  /* Allocate the backend object */
  x11_back = (x11_backend_t *)calloc(1, sizeof(x11_backend_t));
  if (x11_back == NULL) {
    return false;
  }

  /* Map from X11 windows IDs to window objects */
  x11_back->wid_to_win = ht_new((key_hash_fun_t *)_x11_wid_hash,
                                (key_equal_fun_t *)_x11_wid_equal,
                                32);
  if (x11_back->wid_to_win == NULL) {
    x11_backend_terminate();
    return false;
  }

  /* Connect to X11 server using XCB */
  /* Null means use the DISPLAY variable */
  x11_back->c = xcb_connect(NULL, &x11_back->screen_nbr);
  if ((x11_back->c == NULL) || xcb_connection_has_error(x11_back->c)) {
    x11_backend_terminate();
    return false;
  }

  /* Retrive the underlying file descriptor */
  x11_back->fd = xcb_get_file_descriptor(x11_back->c);

  /* Look for default screen */
  int screen_nbr = x11_back->screen_nbr;
  xcb_screen_iterator_t screen_iter =
    xcb_setup_roots_iterator(xcb_get_setup(x11_back->c));
  for (; screen_iter.rem; --screen_nbr, xcb_screen_next(&screen_iter)) {
    if (screen_nbr == 0) {
      x11_back->screen = screen_iter.data;
      break;
    }
  }

  /* Setup the XKB extension */
  xcb_xkb_use_extension_cookie_t xkb_cookie =
    xcb_xkb_use_extension(x11_back->c,
                          XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
  xcb_xkb_use_extension_reply_t *xkb_reply =
    xcb_xkb_use_extension_reply(x11_back->c, xkb_cookie, NULL);
  if (xkb_reply == NULL) {
    x11_backend_terminate();
    return false;
  }
  free(xkb_reply);
  x11_back->_XCB_XKB_EVENT =
    xcb_get_extension_data(x11_back->c, &xcb_xkb_id)->first_event;

  /* Retrieve the keysyms and mapping */
  x11_keyboard_refresh();
  if (x11_back->xkb_get_map_reply == NULL) {
    x11_backend_terminate();
    return false;
  }


  xcb_xkb_per_client_flags_cookie_t xkb_cf_cookie =
    xcb_xkb_per_client_flags(x11_back->c, XCB_XKB_ID_USE_CORE_KBD,
                             XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                             XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                             0, 0, 0);

  xcb_xkb_per_client_flags_reply_t *xkb_cf_reply =
    xcb_xkb_per_client_flags_reply (x11_back->c, xkb_cf_cookie, NULL);

  free(xkb_cf_reply);


  /* Setup XKB events to listen */

  uint16_t which =
    XCB_XKB_EVENT_TYPE_NEW_KEYBOARD_NOTIFY |
    XCB_XKB_EVENT_TYPE_MAP_NOTIFY |
    XCB_XKB_EVENT_TYPE_STATE_NOTIFY;

  uint16_t nkb_details =
    XCB_XKB_NKN_DETAIL_KEYCODES;

  uint16_t map_parts =
    XCB_XKB_MAP_PART_KEY_TYPES |
    XCB_XKB_MAP_PART_KEY_SYMS |
    XCB_XKB_MAP_PART_MODIFIER_MAP |
    XCB_XKB_MAP_PART_EXPLICIT_COMPONENTS |
    XCB_XKB_MAP_PART_KEY_ACTIONS |
    XCB_XKB_MAP_PART_VIRTUAL_MODS |
    XCB_XKB_MAP_PART_VIRTUAL_MOD_MAP;

  uint16_t state_parts =
    XCB_XKB_STATE_PART_MODIFIER_STATE |
    XCB_XKB_STATE_PART_MODIFIER_BASE |
    XCB_XKB_STATE_PART_MODIFIER_LATCH |
    XCB_XKB_STATE_PART_MODIFIER_LOCK |
    XCB_XKB_STATE_PART_GROUP_STATE |
    XCB_XKB_STATE_PART_GROUP_BASE |
    XCB_XKB_STATE_PART_GROUP_LATCH |
    XCB_XKB_STATE_PART_GROUP_LOCK;

  xcb_xkb_select_events_details_t ed = {
    .affectNewKeyboard = nkb_details,
    .newKeyboardDetails = nkb_details,
    .affectState = state_parts,
    .stateDetails = state_parts,
  };

  xcb_xkb_select_events_aux(x11_back->c, XCB_XKB_ID_USE_CORE_KBD,
    which, 0 /* clear */, 0 /* select_all */, map_parts, map_parts, &ed);



  /* TODO: only if shm is available */
  /* Query SHM extension */
  xcb_shm_query_version_cookie_t cookie =
    xcb_shm_query_version(x11_back->c);
  xcb_shm_query_version_reply_t *shm_reply =
    xcb_shm_query_version_reply(x11_back->c, cookie, NULL);
  if (!shm_reply || !shm_reply->shared_pixmaps) {
    x11_back->has_shm = 0;
  } else {
    x11_back->has_shm = 1;
    x11_back->_XCB_SHM_COMPLETION =
      xcb_get_extension_data(x11_back->c, &xcb_shm_id)->first_event +
        XCB_SHM_COMPLETION;
  }
  if (shm_reply) {
    free(shm_reply);
  }

  /* Query atoms */
  x11_back->WM_PROTOCOLS =
    _x11_query_atom(x11_back->c, "WM_PROTOCOLS");
  x11_back->WM_DELETE_WINDOW =
    _x11_query_atom(x11_back->c, "WM_DELETE_WINDOW");
  x11_back->WM_TAKE_FOCUS =
    _x11_query_atom(x11_back->c, "WM_TAKE_FOCUS");
  x11_back->WM_CLIENT_LEADER =
    _x11_query_atom(x11_back->c, "WM_CLIENT_LEADER");
  x11_back->UTF8_STRING =
    _x11_query_atom(x11_back->c, "UTF8_STRING");
  x11_back->_NET_WM_NAME =
    _x11_query_atom(x11_back->c, "_NET_WM_NAME");
  x11_back->_NET_WM_ICON_NAME =
    _x11_query_atom(x11_back->c, "_NET_WM_ICON_NAME");
  x11_back->_NET_WM_PID =
    _x11_query_atom(x11_back->c, "_NET_WM_PID");
  x11_back->_NET_WM_PING =
    _x11_query_atom(x11_back->c, "_NET_WM_PING");
  x11_back->_NET_WM_SYNC_REQUEST =
    _x11_query_atom(x11_back->c, "_NET_WM_SYNC_REQUEST");
  x11_back->_NET_WM_USER_TIME_WINDOW =
    _x11_query_atom(x11_back->c, "_NET_WM_USER_TIME_WINDOW");
  x11_back->_MOTIF_WM_HINTS =
    _x11_query_atom(x11_back->c, "_MOTIF_WM_HINTS");
  x11_back->_NET_WM_WINDOW_TYPE =
    _x11_query_atom(x11_back->c, "_NET_WM_WINDOW_TYPE");
  x11_back->_NET_WM_WINDOW_TYPE_SPLASH =
    _x11_query_atom(x11_back->c, "_NET_WM_WINDOW_TYPE_SPLASH");

  return true;
}

void
x11_backend_terminate(
  void)
{
  if (x11_back == NULL) {
    return;
  }

  if (x11_back->xkb_get_map_reply != NULL) {
    free(x11_back->xkb_get_map_reply);
  }

  if (x11_back->c != NULL) {
    xcb_disconnect(x11_back->c);
  }

  if (x11_back->wid_to_win != NULL) {
// TODO: iterate to close windows
    ht_delete(x11_back->wid_to_win);
  }

  free(x11_back);

  x11_back = NULL;
}

void
x11_backend_add_window(
  x11_window_t *w)
{
  assert(x11_back != NULL);
  assert(w != NULL);
  assert(w->wid != XCB_WINDOW_NONE);

  ht_add(x11_back->wid_to_win, (void *)&(w->wid), (void *)w);
}

void
x11_backend_remove_window(
  const x11_window_t *w)
{
  assert(x11_back != NULL);
  assert(w != NULL);
  assert(w->wid != XCB_WINDOW_NONE);

  ht_remove(x11_back->wid_to_win, (void *)&(w->wid));
}

x11_window_t *
x11_backend_get_window(
  xcb_window_t wid)
{
  assert(x11_back != NULL);
  assert(wid != XCB_WINDOW_NONE);

  return (x11_window_t *)ht_find(x11_back->wid_to_win, (void *)&wid);
}

void
x11_backend_set_listener(
  event_listener_t *listener)
{
  assert(x11_back != NULL);

  if (listener != NULL) {
    assert(listener->process_event != NULL);
  }
  x11_back->listener = listener;
}

event_listener_t *
x11_backend_get_listener(
  void)
{
  assert(x11_back != NULL);

  return x11_back->listener;
}

void
_x11_present_window(
  x11_window_t *w)
{
  assert(x11_back != NULL);

  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_PRESENT;
    evt.time = x11_get_time();
    evt.target = (void *)w;
    event_notify(x11_back->listener, &evt);
  }
}

static void
_x11_render_all_windows(
  void)
{
  assert(x11_back != NULL);

  event_t evt;
  x11_window_t *w = NULL;
  hashtable_iterator_t *i = NULL;

  evt.type = EVENT_FRAME_CYCLE;
  evt.time = x11_get_time();
  evt.target = (void *)NULL;
  event_notify(x11_back->listener, &evt);

  evt.type = EVENT_FRAME;
  i = ht_get_iterator(x11_back->wid_to_win);
  if (i != NULL) {
    while ((w = (x11_window_t *)ht_iterator_next(i)) != NULL) {
      if (w->base.visible == true) {
        evt.target = (void *)w;
        if (event_notify(x11_back->listener, &evt)) {
          _x11_present_window(w);
        }
      }
    }
    ht_free_iterator(i);
  }
}

static void
_x11_update_modifiers(
  key_code_t keycode, /* translated */
  uint8_t event_type)
{
  assert(x11_back != NULL);

  key_modifier_t modifier = MOD_NONE;
  switch (keycode) {
    case KEY_LSHIFT: case KEY_RSHIFT:
      modifier = MOD_SHIFT;
      break;
    case KEY_LALT: case KEY_RALT:
      modifier = MOD_ALT;
      break;
    case KEY_LCONTROL: case KEY_RCONTROL:
      modifier = MOD_CTRL;
      break;
    case KEY_LMETA: case KEY_RMETA:
      modifier = MOD_META;
      break;
    case KEY_CAPSLOCK:
      modifier = MOD_CAPSLOCK;
      break;
    default:
      return;
  }
  if (event_type == XCB_KEY_PRESS) {
    x11_back->modifiers |= modifier;
  } else {
    x11_back->modifiers &= ~modifier;
  }
}

void
x11_backend_run(
  void)
{
  assert(x11_back != NULL);
  assert(x11_back->running == false);

  x11_window_t *w = NULL;
  xcb_event_t e;
  uint8_t event_type;
  event_t evt;
  fd_set fds;

  struct timespec ts_current = { .tv_sec = 0, .tv_nsec = 0 };
  struct timespec ts_next_frame = { .tv_sec = 0, .tv_nsec = 0 };
  struct timeval tv_frame_timeout = { .tv_sec = 0, .tv_usec = 0 };

  FD_ZERO(&fds);

  clock_gettime(CLOCK_MONOTONIC, &ts_next_frame);

  x11_back->running = true;

  while (x11_back->running) {

    /* Handle events */
    if ((e.generic = xcb_poll_for_event(x11_back->c))) {

      /* Top bit = from SendEvent request */
      event_type = e.generic->response_type & ~0x80;

      switch (event_type) {

        case XCB_KEY_PRESS:
        case XCB_KEY_RELEASE: {
          key_code_t kc = x11_keyboard_translate_keycode(e.key_press->detail);
          _x11_update_modifiers(kc, event_type);
          w = x11_backend_get_window(e.key_press->event);
          if (w != NULL) {
            xcb_keysym_t k = x11_keysym_of_event(e.key_press);
            evt.type = EVENT_KEY;
            evt.time = x11_get_time();
            evt.target = (void *)w;
            evt.desc.key.code = kc;
            evt.desc.key.char_ = x11_keysym_to_unicode(k);
            evt.desc.key.dead = x11_keysym_is_dead(k);
            evt.desc.key.modifiers = x11_back->modifiers;
            evt.desc.key.state =
              (event_type == XCB_KEY_PRESS) ? KEY_DOWN : KEY_UP;
            event_notify(x11_back->listener, &evt);
          }
          break;
        }

        case XCB_BUTTON_PRESS:
        case XCB_BUTTON_RELEASE:
          w = x11_backend_get_window(e.button_press->event);
          if (w != NULL) {
            evt.type = EVENT_BUTTON;
            evt.time = x11_get_time();
            evt.target = (void *)w;
            evt.desc.button.x = e.button_press->event_x;
            evt.desc.button.y = e.button_press->event_y;
            evt.desc.button.button = (button_t)e.button_press->detail;
            evt.desc.button.state =
              (event_type == XCB_BUTTON_PRESS) ? BUTTON_DOWN : BUTTON_UP;
            event_notify(x11_back->listener, &evt);
          }
          break;

        case XCB_MOTION_NOTIFY:
          w = x11_backend_get_window(e.motion_notify->event);
          if (w != NULL) {
            evt.type = EVENT_CURSOR;
            evt.time = x11_get_time();
            evt.target = (void *)w;
            evt.desc.cursor.x = e.motion_notify->event_x;
            evt.desc.cursor.y = e.motion_notify->event_y;
            event_notify(x11_back->listener, &evt);
          }
          break;

        case XCB_ENTER_NOTIFY:
          break;

        case XCB_LEAVE_NOTIFY:
          break;

        case XCB_FOCUS_IN:
        case XCB_FOCUS_OUT:
          /* Ignore grabs */
          if (e.focus_in->mode != XCB_NOTIFY_MODE_NORMAL) {
            break;
          }
          w = x11_backend_get_window(e.focus_in->event);
          if (w != NULL) {
            evt.type = EVENT_FOCUS;
            evt.time = x11_get_time();
            evt.target = (void *)w;
            evt.desc.focus.inout =
              (event_type == XCB_FOCUS_IN) ? FOCUS_IN : FOCUS_OUT;
            event_notify(x11_back->listener, &evt);
          }
          break;

        case XCB_KEYMAP_NOTIFY:
          break;

        case XCB_EXPOSE:
          break;

        case XCB_GRAPHICS_EXPOSURE:
          break;

        case XCB_NO_EXPOSURE:
          break;

        case XCB_VISIBILITY_NOTIFY:
          break;

        case XCB_CREATE_NOTIFY:
          break;

        case XCB_DESTROY_NOTIFY:
          break;

        case XCB_UNMAP_NOTIFY:
          break;

        case XCB_MAP_NOTIFY:
          w = x11_backend_get_window(e.map_notify->window);
          _x11_present_window(w);
          break;

        case XCB_MAP_REQUEST:
          break;

        case XCB_REPARENT_NOTIFY:
          break;

        case XCB_CONFIGURE_NOTIFY:
          w = x11_backend_get_window(e.configure_notify->window);
          if (w != NULL) {
            if (w->base.width != e.configure_notify->width ||
                w->base.height != e.configure_notify->height) {
              w->base.width = e.configure_notify->width;
              w->base.height = e.configure_notify->height;
              evt.type = EVENT_RESIZE;
              evt.time = x11_get_time();
              evt.target = (void *)w;
              evt.desc.resize.width = w->base.width;
              evt.desc.resize.height = w->base.height;
              event_notify(x11_back->listener, &evt);
            }
            if (w->base.x != e.configure_notify->x ||
                w->base.y != e.configure_notify->y) {
              w->base.x = e.configure_notify->x;
              w->base.y = e.configure_notify->y;
              evt.type = EVENT_MOVE;
              evt.time = x11_get_time();
              evt.target = (void *)w;
              evt.desc.move.x = w->base.x;
              evt.desc.move.y = w->base.y;
              event_notify(x11_back->listener, &evt);
            }
          }
          break;

        case XCB_CONFIGURE_REQUEST:
          break;

        case XCB_GRAVITY_NOTIFY:
          break;

        case XCB_RESIZE_REQUEST:
          break;

        case XCB_CIRCULATE_NOTIFY:
          break;

        case XCB_CIRCULATE_REQUEST:
          break;

        case XCB_PROPERTY_NOTIFY:
          break;

        case XCB_SELECTION_CLEAR:
          break;

        case XCB_SELECTION_REQUEST:
          break;

        case XCB_SELECTION_NOTIFY:
          break;

        case XCB_COLORMAP_NOTIFY:
          break;

        case XCB_CLIENT_MESSAGE:
          if (e.client_message->type == x11_back->WM_PROTOCOLS) {
            uint32_t proto = e.client_message->data.data32[0];
            if (proto == x11_back->WM_DELETE_WINDOW) {
              w = x11_backend_get_window(e.client_message->window);
              if (w != NULL) {
                evt.type = EVENT_CLOSE;
                evt.time = x11_get_time();
                evt.target = (void *)w;
                event_notify(x11_back->listener, &evt);
              }
            } else if (proto == x11_back->_NET_WM_PING) {
              e.client_message->window = x11_back->screen->root;
              xcb_send_event(x11_back->c, 0,
                             x11_back->screen->root,
                             XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
                             XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT,
                             (const char *)e.client_message);
              xcb_flush(x11_back->c); // MUST flush
            } else if (proto == x11_back->WM_TAKE_FOCUS) {

            } else if (proto == x11_back->_NET_WM_SYNC_REQUEST) {

            }
          }
          break;

        case XCB_MAPPING_NOTIFY:
          break;

        case XCB_GE_GENERIC:
          break;

        default:
          if (event_type == x11_back->_XCB_XKB_EVENT) {
            switch (e.xkb_any->xkbType) {
              case XCB_XKB_NEW_KEYBOARD_NOTIFY:
                if (e.xkb_new_keyboard_notify->changed &
                      XCB_XKB_NKN_DETAIL_KEYCODES) {
                  x11_keyboard_refresh();
                }
                break;
              case XCB_XKB_MAP_NOTIFY:
                x11_keyboard_refresh();
                break;
              case XCB_XKB_STATE_NOTIFY:
                _x11_update_modifiers(
                  x11_keyboard_translate_keycode(e.xkb_state_notify->keycode),
                  e.xkb_state_notify->eventType);
                break;
            }
            break;
          }
          break;
      }

      free(e.generic); // Beware, when using extended events (generic), have to free more data

    } else {
      /* Update remaining time to wait */
      clock_gettime(CLOCK_MONOTONIC, &ts_current);
      tv_frame_timeout.tv_usec =
        (ts_next_frame.tv_sec - ts_current.tv_sec) * 1000000 +
        (ts_next_frame.tv_nsec - ts_current.tv_nsec) / 1000;

      /* Wait for new events or frame */
      if (tv_frame_timeout.tv_usec > 0) {
        FD_SET(x11_back->fd, &fds);
      };
      if ((tv_frame_timeout.tv_usec <= 0) ||
          (select(1, &fds, NULL, NULL, &tv_frame_timeout) == 0)) {
        _x11_render_all_windows();

        /* Compute time until next frame, skip frames if needed */
        do {
          ts_next_frame.tv_nsec += 1000000000 / 60;
          if (ts_next_frame.tv_nsec >= 1000000000) {
            ts_next_frame.tv_nsec -= 1000000000;
            ts_next_frame.tv_sec += 1;
          }
        } while ((ts_next_frame.tv_sec < ts_current.tv_sec) ||
                 ((ts_next_frame.tv_sec == ts_current.tv_sec) &&
                  (ts_next_frame.tv_nsec < ts_current.tv_nsec)));
      }
    }
  }
}

void
x11_backend_stop(
  void)
{
  assert(x11_back != NULL);

  x11_back->running = false;
}

#else

const int x11_backend = 0;

#endif /* HAS_X11 */
