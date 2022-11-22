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

#include "wl_target.h"

wl_target_t *
wl_target_create(
  void *dummy)
{
  return NULL;
}

void
wl_target_destroy(
  wl_target_t *target)
{
}

#else

const int wl_target = 0;

#endif /* HAS_WAYLAND */
