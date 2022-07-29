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

#include "pixmap.h"
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

static color_t_ _pattern_evaluate_pixel(
  const pattern_t *pattern,
  int32_t x,
  int32_t y)
{
  assert(pattern != NULL);

  return pixmap_at(pattern->image, y, x);
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

  //Bilinear interpolation :
  double uvx = p.x;
  double uvy = p.y;
  int32_t width = pattern->image.width;
  int32_t height = pattern->image.height;
  int32_t pt_x = (int32_t)uvx;
  int32_t pt_y = (int32_t)uvy;
  double dec_x = uvx - (double)pt_x;
  double dec_y = uvy - (double)pt_y;

  color_t_ col11 =
    _pattern_evaluate_pixel(pattern, pt_x, pt_y);
  color_t_ col21 =
    (pt_x + 1 < width) ?
    _pattern_evaluate_pixel(pattern, pt_x + 1, pt_y) :
    col11;
  color_t_ col12 =
    (pt_y + 1 < height) ?
    _pattern_evaluate_pixel(pattern, pt_x, pt_y + 1) :
    col11;
  color_t_ col22 =
    ((pt_x + 1 < width) && (pt_y + 1 < height)) ?
    _pattern_evaluate_pixel(pattern, pt_x + 1, pt_y + 1) :
    col11;

  double w11 = (1.0 - dec_x) * (1.0 - dec_y);
  double w12 = (1.0 - dec_x) - w11;
  double w21 = dec_x * (1.0 - dec_y);
  double w22 = dec_x - w21;

  uint8_t r =
    col11.r * w11 + col12.r * w12 +
    col21.r * w21 + col22.r * w22;
  uint8_t g =
    col11.g * w11 + col12.g * w12 +
    col21.g * w21 + col22.g * w22;
  uint8_t b =
    col11.b * w11 + col12.b * w12 +
    col21.b * w21 + col22.b * w22;
  uint8_t a =
    col11.a * w11 + col12.a * w12 +
    col21.a * w21 + col22.a * w22;

  return color(a, r, g, b);
}

static void (*_pattern_destroy_callback)(pattern_t *) = NULL;

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
