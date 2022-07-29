/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __DRAW_INSTR_H
#define __DRAW_INSTR_H

#include <stdbool.h>

#include "polygon.h"

typedef struct path_fill_instr_t {
  polygon_t *poly;
  bool non_zero;
} path_fill_instr_t;

path_fill_instr_t *
path_fill_instr_create(
  const polygon_t *poly,
  bool non_zero);

void
path_fill_instr_destroy(
  path_fill_instr_t *instr);

#endif /*__DRAW_INSTR_H*/
