/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_BACKEND_H
#define __WL_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"

#include "../event.h"
#include "wl_window.h"

typedef struct wl_backend_t wl_backend_t;

int64_t
_wl_get_time(
  void);

bool
wl_backend_init(
  void);

void
wl_backend_terminate(
  void);

void
wl_backend_add_window(
  wl_window_t *w);

void
wl_backend_set_listener(
  event_listener_t *listener);

event_listener_t *
wl_backend_get_listener(
  void);

void
wl_backend_run(
  void);

void
wl_backend_stop(
  void);

#endif /* __WL_BACKEND_H */
