/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_BACKEND_H
#define __X11_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>

#include "../event.h"
#include "x11_window.h"

typedef struct x11_backend_t x11_backend_t;

int64_t
x11_get_time(
  void);

bool
x11_backend_init(
  void);

void
x11_backend_terminate(
  void);

void
x11_backend_add_window(
  x11_window_t *w);

void
x11_backend_remove_window(
  const x11_window_t *w);

x11_window_t *
x11_backend_get_window(
  xcb_window_t wid);

void
x11_backend_set_listener(
  event_listener_t *listener);

event_listener_t *
x11_backend_get_listener(
  void);

void
x11_backend_run(
  void);

void
x11_backend_stop(
  void);

#endif /* __X11_BACKEND_H */
