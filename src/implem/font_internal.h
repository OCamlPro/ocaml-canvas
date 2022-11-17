/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FONT_INTERNAL_H
#define __FONT_INTERNAL_H

#include "font_desc.h"

typedef struct font_t {
  font_desc_t *font_desc;
} font_t;

#endif /* __FONT_INTERNAL_H */
