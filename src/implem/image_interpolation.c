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
#include <stdbool.h>
#include <assert.h>

#include "color.h"
#include "pixmap.h"

color_t_
interpolation_bilinear(
  const pixmap_t *sp,
  double uvx,
  double uvy)
{
  assert(sp != NULL);
  assert(pixmap_valid(*sp) == true);

  int width = sp->width;
  int height = sp->height;

  int32_t pt_x = (int32_t)uvx;
  int32_t pt_y = (int32_t)uvy;

  double dec_x = uvx - (double)pt_x;
  double dec_y = uvy - (double)pt_y;

  color_t_ col11 =
    pixmap_at(*sp, pt_y, pt_x);
  color_t_ col11_out = col11;
  col11_out.a = 0;
  color_t_ col21 =
    (pt_x + 1 < width) ?
    pixmap_at(*sp, pt_y, pt_x + 1) :
    col11_out;
  color_t_ col12 =
    (pt_y + 1 < height) ?
    pixmap_at(*sp, pt_y + 1, pt_x) :
    col11_out;
  color_t_ col22 =
    ((pt_x + 1 < width) && (pt_y + 1 < height)) ?
    pixmap_at(*sp, pt_y + 1, pt_x + 1) :
    col11_out;

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

static double
_interpolation_cubic_h(
  double t,
  double a)
{
  double abs_t = (t >= 0.0) ? t : -t;
  double abs_t_s = abs_t * abs_t;
  double abs_t_c = abs_t_s * abs_t;

  if (abs_t >= 2.0) {
    return 0.0;
  } else if (abs_t > 1.0) {
    return a * abs_t_c - 5.0 * a * abs_t_s + 8.0 * a * abs_t - 4.0 * a;
  } else {
    return (a + 2.0) * abs_t_c - (a + 3.0) * abs_t_s + 1.0;
  }
}

color_t_
interpolation_cubic(
  const pixmap_t *image,
  double uvx,
  double uvy)
{
  assert(image != NULL);
  assert(pixmap_valid(*image) == true);

  int floor_x = (int)(uvx);
  int floor_y = (int)(uvy);

  double x_vals[4] = {
    (double)(floor_x - 1) - uvx, (double)(floor_x) - uvx,
    (double)(floor_x + 1) - uvx, (double)(floor_x + 2) - uvx};

  double y_vals[4] = {
    (double)(floor_y - 1) - uvy, (double)(floor_y) - uvy,
    (double)(floor_y + 1) - uvy, (double)(floor_y + 2) - uvy};

  double r = 0.0, g = 0.0, b = 0.0, a = 0.0;

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {

      double x_coeff = _interpolation_cubic_h(x_vals[i], -0.75);
      double y_coeff = _interpolation_cubic_h(y_vals[j], -0.75);

      color_t_ sample =
        pixmap_at(*image,
                  max(0, min(image->height - 1, (int)(uvy + y_vals[j]))),
                  max(0, min(image->width - 1, (int)(uvx + x_vals[i]))));

      r += (x_coeff * y_coeff * (double)sample.r);
      g += (x_coeff * y_coeff * (double)sample.g);
      b += (x_coeff * y_coeff * (double)sample.b);
      a += (x_coeff * y_coeff * (double)sample.a);
    }
  }

  r = min(max(r, 0.0), 255.0);
  g = min(max(g, 0.0), 255.0);
  b = min(max(b, 0.0), 255.0);
  a = min(max(a, 0.0), 255.0);

  return color((uint8_t)a, (uint8_t)r, (uint8_t)g, (uint8_t)b);
}
