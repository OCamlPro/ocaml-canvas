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

#include <stdlib.h>
#include <assert.h>

#include <windows.h>

#include "gdi_target.h"

gdi_target_t *
gdi_target_create(
  HWND hwnd)
{
  assert(hwnd != NULL);
  gdi_target_t *target = (gdi_target_t *)calloc(1, sizeof(gdi_target_t));
  if (target == NULL) {
    return NULL;
  }
  target->hwnd = hwnd;
  return target;
}

void
gdi_target_destroy(
  gdi_target_t *target)
{
  assert(target != NULL);
  free(target);
}

#else

const int gdi_target = 0;

#endif /* HAS_GDI */
