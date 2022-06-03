/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_BACKEND_H
#define __GDI_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#include <windows.h>

#include "../event.h"
#include "gdi_window.h"

typedef struct gdi_backend_t gdi_backend_t;

int64_t
gdi_get_time(
  void);

bool
gdi_backend_init(
  void);

void
gdi_backend_terminate(
  void);

void
gdi_backend_add_window(
  gdi_window_t *w);

void
gdi_backend_remove_window(
  const gdi_window_t *w);

gdi_window_t *
gdi_backend_get_window(
  HWND hwnd);

void
gdi_backend_set_listener(
  event_listener_t *listener);

event_listener_t *
gdi_backend_get_listener(
  void);

void
gdi_backend_run(
  void);

void
gdi_backend_stop(
  void);

#endif /* __GDI_BACKEND_H */
