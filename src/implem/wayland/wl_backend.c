/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include <stddef.h>
#include <stdbool.h>

#include "../event.h"
#include "wl_backend_internal.h"
#include "wl_window_internal.h"

wl_backend_t *wl_back = NULL;

int64_t
wl_get_time(
  void)
{
  return 0;
}

bool
wl_backend_init(
  void)
{
  return false;
}

void
wl_backend_terminate(
  void)
{
}

void
wl_backend_add_window(
  wl_window_t *w)
{
}

void
wl_backend_remove_window(
  const wl_window_t *w)
{
}

wl_window_t *
wl_backend_get_window(
  int wid)
{
  return NULL;
}

void
wl_backend_set_listener(
  event_listener_t *listener)
{
}

event_listener_t *
wl_backend_get_listener(
  void)
{
  return NULL;
}

void
wl_backend_run(
  void)
{
}

void
wl_backend_stop(
  void)
{
}

#else

const int wl_backend = 0;

#endif /* HAS_WAYLAND */
