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
#include <stdint.h>
#include <memory.h>
#include <assert.h>

#include "util.h"
#include "color.h"
#include "pixmap.h"

void
pixmap_blit(
  pixmap_t *dp,
  int32_t dx,
  int32_t dy,
  const pixmap_t *sp,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(dp != NULL);
  assert(pixmap_valid(*sp));
  assert(sp != NULL);
  assert(pixmap_valid(*dp));
  assert(width > 0);
  assert(height > 0);

  adjust_blit_info(dp->width, dp->height, dx, dy,
                   sp->width, sp->height, sx, sy,
                   width, height);

  if ((width > 0) && (height > 0)) {
    for (int32_t i = 0; i < height; ++i) {
      memcpy(&pixmap_at(*dp, dy + i, dx),
             &pixmap_at(*sp, sy + i, sx),
             width * COLOR_SIZE);
    }
  }
}
