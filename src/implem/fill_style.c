/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include "gradient.h"
#include "fill_style.h"

void
fill_style_destroy(
  fill_style_t *fs)
{
  assert(fs != NULL);

  if (fs->fill_type == FILL_TYPE_GRADIENT) {
    gradient_release(fs->content.gradient);
  }
}

fill_style_t
fill_style_copy(
  const fill_style_t *fs)
{
  assert(fs != NULL);

  fill_style_t fsc = (fill_style_t)*fs;
  if (fsc.fill_type == FILL_TYPE_GRADIENT) {
    gradient_retain(fsc.content.gradient);
  }
  return fsc;
}
