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

#include "object.h"
#include "pixmap.h"
#include "image_interpolation.h"
#include "pattern.h"
#include "pattern_internal.h"

IMPLEMENT_OBJECT_METHODS(pattern_t, pattern, _pattern_destroy)

pattern_t *
pattern_create(
  const pixmap_t *image,
  pattern_repeat_t repeat)
{
  assert(pixmap_valid(*image));

  pattern_t *p = pattern_alloc();
  if (p == NULL) {
    return NULL;
  }

  p->repeat = repeat;
  p->image = pixmap_copy(*image);
  if (pixmap_valid(p->image) == false) {
    free(p);
    return NULL;
  }

  return p;
}

color_t_
pattern_evaluate_pos(
  const pattern_t *pattern,
  double pos_x,
  double pos_y,
  const transform_t *inverse)
{
  assert(pattern != NULL);
  assert(inverse != NULL);

  point_t p = point(pos_x, pos_y);
  transform_apply(inverse, &p);

  switch (pattern->repeat) {
    case PATTERN_NO_REPEAT:
      p.x = min(max(p.x, 0), pattern->image.width - 1);
      p.y = min(max(p.y, 0), pattern->image.height - 1);
      break;
    case PATTERN_REPEAT_X:
      p.x -= pattern->image.width * floor(p.x / pattern->image.width);
      p.y = min(max(p.y, 0), pattern->image.height - 1);
      break;
    case PATTERN_REPEAT_Y:
      p.x = min(max(p.x, 0), pattern->image.width - 1);
      p.y -= pattern->image.height * floor(p.y / pattern->image.height);
      break;
    case PATTERN_REPEAT_XY:
      p.x -= pattern->image.width * floor(p.x / pattern->image.width);
      p.y -= pattern->image.height * floor(p.y / pattern->image.height);
      break;
    default:
      break;
  }

  return interpolation_cubic(&pattern->image, p.x, p.y);
}

static void (*_pattern_destroy_callback)(pattern_t *) = NULL;

void
pattern_set_destroy_callback(
  void (*callback_function)(pattern_t *))
{
  _pattern_destroy_callback = callback_function;
}

static void
_pattern_destroy(
  pattern_t *pattern)
{
  assert(pattern != NULL);

  if (_pattern_destroy_callback != NULL) {
    _pattern_destroy_callback(pattern);
  }

  pixmap_destroy(pattern->image);

  free(pattern);
}
