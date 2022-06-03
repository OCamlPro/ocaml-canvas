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

#include <stdlib.h>
#include <assert.h>

#include <wayland-client.h>

#include "wl_target.h"

wl_target_t *
wl_target_create(
  struct wl_shm *wl_shm,
  struct wl_surface *wl_surface)
{
  wl_target_t *target = (wl_target_t *)calloc(1, sizeof(wl_target_t));
  if (target == NULL) {
    return NULL;
  }
  target->wl_shm = wl_shm;
  target->wl_surface = wl_surface;
  return target;
}

void
wl_target_destroy(
  wl_target_t *target)
{
  assert(target != NULL);
  free(target);
}

#else

const int wl_target = 0;

#endif /* HAS_WAYLAND */
