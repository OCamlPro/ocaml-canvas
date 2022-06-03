/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FONT_DESC_INTERNAL_H
#define __FONT_DESC_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include "font_desc.h"

typedef struct font_desc_t {
  const char *family;
  double size;
  font_slant_t slant;
  int32_t weight;
  double scale;
} font_desc_t;

#endif /* __FONT_DESC_INTERNAL_H */
