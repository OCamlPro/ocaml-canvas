/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_TARGET_H
#define __GDI_TARGET_H

#include <windows.h>

typedef struct gdi_target_t {
  HWND hwnd;
} gdi_target_t;

gdi_target_t *
gdi_target_create(
  HWND hwnd);

void
gdi_target_destroy(
  gdi_target_t *target);

#endif /* __GDI_TARGET_H */
