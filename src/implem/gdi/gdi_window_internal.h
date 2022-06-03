/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_WINDOW_INTERNAL_H
#define __GDI_WINDOW_INTERNAL_H

#include <windows.h>

#include "../window_internal.h"

typedef struct gdi_window_t {

  /* Common to all windows */
  window_t base;

  /* Specific to GDI windows */
  HWND hwnd;

} gdi_window_t;

#endif /* __GDI_WINDOW_INTERNAL_H */
