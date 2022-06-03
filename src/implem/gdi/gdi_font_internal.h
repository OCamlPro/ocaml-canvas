/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_FONT_INTERNAL_H
#define __GDI_FONT_INTERNAL_H

#include <windows.h>

#include "../font_internal.h"

typedef struct gdi_font_t {

  /* Common to all fonts */
  font_t base;

  /* Specific to GDI fonts */
  HFONT hfont;
  HFONT holdfont;
  HDC hdc;

} gdi_font_t;

#endif /* __GDI_FONT_INTERNAL_H */
