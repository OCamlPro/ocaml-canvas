/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_FONT_INTERNAL_H
#define __QTZ_FONT_INTERNAL_H

#include <CoreText/CoreText.h>

#include "../font_internal.h"

typedef struct qtz_font_t {

  /* Common to all fonts */
  font_t base;

  /* Specific to QTZ fonts */
  CTFontRef font;

} qtz_font_t;

#endif /* __QTZ_FONT_INTERNAL_H */
