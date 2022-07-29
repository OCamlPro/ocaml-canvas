/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __UNX_FONT_INTERNAL_H
#define __UNX_FONT_INTERNAL_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../font_internal.h"

typedef struct unx_font_t {

  /* Common to all fonts */
  font_t base;

  /* Specific to FreeType fonts */
  FT_Face ft_face;

} unx_font_t;

#endif /* __UNX_FONT_INTERNAL_H */
