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
#include <assert.h>

#include <xcb/xcb.h>

#include "x11_target.h"

x11_target_t *
x11_target_create(
  xcb_window_t wid)
{
  x11_target_t *target = (x11_target_t *)calloc(1, sizeof(x11_target_t));
  if (target == NULL) {
    return NULL;
  }
  target->wid = wid;
  return target;
}

void
x11_target_destroy(
  x11_target_t *target)
{
  assert(target != NULL);
  free(target);
}

#else

const int x11_target = 0;

#endif /* HAS_X11 */
