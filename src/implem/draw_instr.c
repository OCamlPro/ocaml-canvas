/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "polygon.h"
#include "draw_instr.h"

path_fill_instr_t *
path_fill_instr_create(
  const polygon_t *poly,
  bool non_zero)
{
  assert(poly != NULL);

  path_fill_instr_t *instr =
    (path_fill_instr_t *)calloc(1, sizeof(path_fill_instr_t));
  if (instr == NULL) {
    return NULL;
  }

  instr->poly = polygon_copy(poly);
  instr->non_zero = non_zero;

  return instr;
}

void
path_fill_instr_destroy(
  path_fill_instr_t *instr)
{
  assert(instr != NULL);
  assert(instr->poly != NULL);

  polygon_destroy(instr->poly);
  free(instr);
}
