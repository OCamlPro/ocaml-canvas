/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PATTERN_INTERNAL_H
#define __PATTERN_INTERNAL_H

#include "object.h"
#include "pixmap.h"
#include "pattern.h"

typedef struct pattern_t {
  INHERITS_OBJECT;
  pattern_repeat_t repeat;
  pixmap_t image;
} pattern_t;

#endif /* __PATTERN_INTERNAL_H */
