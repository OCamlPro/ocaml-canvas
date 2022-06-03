/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_BACKEND_INTERNAL_H
#define __X11_BACKEND_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xcb/xcb_keysyms.h>

#include "../hashtable.h"
#include "../event.h"

typedef struct x11_keysyms_t {
  xcb_keysym_t *keysyms; /* points into xcb_xkb_get_map_reply_t */
  uint8_t *grp_kt;
  uint8_t grp_info;
  uint8_t grp_width;
} x11_keysyms_t;

typedef struct x11_keytypes_t {
  xcb_xkb_kt_map_entry_t *keytypes; /* points into xcb_xkb_get_map_reply_t */
  uint8_t mods_mask;
  uint8_t nb_entries;
} x11_keytypes_t;

typedef struct x11_backend_t {

  xcb_connection_t *c;
  int fd; /* file descriptor of the connection */

  int screen_nbr;
  xcb_screen_t *screen;

  xcb_xkb_get_map_reply_t *xkb_get_map_reply;
  x11_keytypes_t keytypes[256];
  x11_keysyms_t keysyms[256]; /* maps X11 keycodes to X11 keysyms */
  key_code_t keycodes[256]; /* maps X11 keycodes to universal key codes */
  key_modifier_t modifiers;

  hashtable_t *wid_to_win;

  bool running;

  event_listener_t *listener;

  bool has_shm;
  uint8_t _XCB_SHM_COMPLETION;

  uint8_t _XCB_XKB_EVENT;

  xcb_atom_t WM_PROTOCOLS;
  xcb_atom_t WM_DELETE_WINDOW;
  xcb_atom_t WM_TAKE_FOCUS;
  xcb_atom_t WM_CLIENT_LEADER;
  xcb_atom_t UTF8_STRING;
  xcb_atom_t _NET_WM_NAME;
  xcb_atom_t _NET_WM_ICON_NAME;
  xcb_atom_t _NET_WM_PID;
  xcb_atom_t _NET_WM_PING;
  xcb_atom_t _NET_WM_SYNC_REQUEST;
  xcb_atom_t _NET_WM_USER_TIME_WINDOW;
  xcb_atom_t _MOTIF_WM_HINTS;
  xcb_atom_t _NET_WM_WINDOW_TYPE;
  xcb_atom_t _NET_WM_WINDOW_TYPE_SPLASH;

// glx stuff
} x11_backend_t;

extern x11_backend_t *x11_back;

#endif /* __X11_BACKEND_INTERNAL_H */
