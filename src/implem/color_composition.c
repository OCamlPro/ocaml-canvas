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
#include <math.h>
#include <assert.h>

#include "util.h"
#include "color.h"
#include "color_composition.h"

color_t_
comp_source_over(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  if (draw_alpha == 255) {
    return src;
  }
  int alpha = draw_alpha + dst.a - (draw_alpha * dst.a) / 255;
  color_t_ output = alpha_blend(draw_alpha, dst, src);
  output.a = alpha;
  return output;
}

color_t_
comp_source_in(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ output = color_transparent_black;
  draw_alpha = (dst.a * draw_alpha) / 255;
  if (draw_alpha > 0) {
    output = color(draw_alpha,
                   draw_alpha * src.r / 255,
                   draw_alpha * src.g / 255,
                   draw_alpha * src.b / 255);
  }
  return output;
}

color_t_
comp_source_out(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ output = color_transparent_black;
  draw_alpha = ((255 - dst.a) * draw_alpha) / 255;
  if (draw_alpha > 0) {
    output = color(draw_alpha,
                            draw_alpha * src.r / 255,
                            draw_alpha * src.g / 255,
                            draw_alpha * src.b / 255);
  }
  return output;
}

color_t_
comp_source_atop(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  int src_alpha = draw_alpha * dst.a / 255;
  int dest_alpha = (255 - draw_alpha) * dst.a / 255;
  int out_alpha = dst.a;
  return color(out_alpha,
               (dest_alpha * dst.r + src_alpha * src.r) / 255,
               (dest_alpha * dst.g + src_alpha * src.g) / 255,
               (dest_alpha * dst.b + src_alpha * src.b) / 255);
}

color_t_
comp_destination_over(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  draw_alpha = (draw_alpha * (255 - dst.a)) / (255);
  return comp_source_over(src, dst, draw_alpha);
}

color_t_
comp_destination_in(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ output = color_transparent_black;
  draw_alpha = (dst.a * draw_alpha) / 255;
  if (draw_alpha > 0) {
    output = color(draw_alpha,
                   draw_alpha * dst.r / 255,
                   draw_alpha * dst.g / 255,
                   draw_alpha * dst.b / 255);
  }
  return output;
}

color_t_
comp_destination_out(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ output = color_transparent_black;
  draw_alpha = (dst.a * (255 - draw_alpha)) / 255;
  if (draw_alpha > 0) {
    output = color(draw_alpha,
                   draw_alpha * dst.r / 255,
                   draw_alpha * dst.g / 255,
                   draw_alpha * dst.b / 255);
  }
  return output;
}

color_t_
comp_destination_atop(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  int src_alpha = draw_alpha * (255 - dst.a) / 255;
  int dest_alpha = draw_alpha * dst.a / 255;
  int out_alpha = draw_alpha;
  return color(out_alpha,
               (dest_alpha * dst.r + src_alpha * src.r) / 255,
               (dest_alpha * dst.g + src_alpha * src.g) / 255,
               (dest_alpha * dst.b + src_alpha * src.b) / 255);
}

color_t_
comp_lighter(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  return color(min(255, draw_alpha + dst.a),
               min(255, src.r + dst.r),
               min(255, src.g + dst.g),
               min(255, src.b + dst.b));
}

color_t_
comp_copy(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  if (draw_alpha == 0) {
    return color_transparent_black;
  }
  return src;
}

color_t_
comp_xor(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  int src_alpha = draw_alpha * (255 - dst.a) / 255;
  int dest_alpha = (255 - draw_alpha) * dst.a / 255;
  int out_alpha = src_alpha + dest_alpha;
  return color(out_alpha,
               (dest_alpha * dst.r + src_alpha * src.r) / 255,
               (dest_alpha * dst.g + src_alpha * src.g) / 255,
               (dest_alpha * dst.b + src_alpha * src.b) / 255);
}

color_t_
comp_multiply(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                src.r * dst.r / 255,
                                src.g * dst.g / 255,
                                src.b * dst.b / 255);
  return comp_source_over(blendedColor, dst, draw_alpha);
}

color_t_
comp_screen(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                255 - (255- src.r) * (255 - dst.r) / 255,
                                255 - (255- src.g) * (255 - dst.g) / 255,
                                255 - (255- src.b) * (255 - dst.b) / 255);
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_overlay_component(
  int bottom,
  int top)
{
  if (bottom < 128) {
    return 2 * top * bottom / 255;
  }
  return 255 - 2 * (255 - top) * (255 - bottom) / 255;
}

color_t_
comp_overlay(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_overlay_component(dst.r, src.r),
                                _comp_overlay_component(dst.g, src.g),
                                _comp_overlay_component(dst.b, src.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

color_t_
comp_darken(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                min(src.r, dst.r),
                                min(src.g, dst.g),
                                min(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

color_t_
comp_lighten(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                max(src.r, dst.r),
                                max(src.g, dst.g),
                                max(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_dodge_component(
  int top,
  int bottom)
{
  if (bottom == 0) {
    return 0;
  }
  if (top == 255) {
    return 255;
  }
  return min(255, ((bottom * 255) / (255 - top)));
}

color_t_
comp_color_dodge(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_dodge_component(dst.r, src.r),
                                _comp_dodge_component(dst.g, src.g),
                                _comp_dodge_component(dst.b, src.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_burn_component(
  int top,
  int bottom)
{
  if (bottom == 255) {
    return 255;
  }
  if (top == 0) {
    return 0;
  }
  return 255 - min(255, ((255 - bottom) * 255) / top);
}

color_t_
comp_color_burn(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_burn_component(dst.r, src.r),
                                _comp_burn_component(dst.g, src.g),
                                _comp_burn_component(dst.b, src.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

color_t_
comp_hard_light(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_overlay_component(src.r, dst.r),
                                _comp_overlay_component(src.g, dst.g),
                                _comp_overlay_component(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_soft_light_component(
  int bottom,
  int top)
{
  if (top < 128) {
    return bottom - ((255 - 2 * top) * bottom * (255 - bottom)) / (255 * 255);
  } else {
    int d = 0;
    if (bottom < 64) {
      d = ((((16 * bottom - 12 * 255) * bottom)
            / 255 + 4 * 255) * bottom) / (255);
    } else {
      d = fastround(255 * sqrt(bottom / 255));
    }
    return bottom + ((2 * top - 255) * (d - bottom)) / 255;
  }
}

color_t_
comp_soft_light(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_soft_light_component(src.r, dst.r),
                                _comp_soft_light_component(src.g, dst.g),
                                _comp_soft_light_component(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_difference_component(
  int a,
  int b)
{
  if (a > b) {
    return a - b;
  }
  else {
    return b - a;
  }
}

color_t_
comp_difference(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_difference_component(src.r, dst.r),
                                _comp_difference_component(src.g, dst.g),
                                _comp_difference_component(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static int
_comp_exclusion_component(
  int a,
  int b)
{
  return a + b - (2 * a * b) / 255;
}


color_t_
comp_exclusion(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blendedColor = color(src.a,
                                _comp_exclusion_component(src.r, dst.r),
                                _comp_exclusion_component(src.g, dst.g),
                                _comp_exclusion_component(src.b, dst.b));
  return comp_source_over(blendedColor, dst, draw_alpha);
}

static double
_comp_lum(
  color_t_ col)
{
  return
    0.3 * col.r / 255.0 +
    0.59 * col.g / 255.0 +
    0.11 * col.b / 255.0;
}

static color_t_
_comp_set_lum(
  color_t_ col,
  double lum)
{
  double original_lum = _comp_lum(col);
  double d = lum - original_lum;
  int intd = fastround(d * 255.0);
  int target_r = col.r + intd;
  int target_g = col.g + intd;
  int target_b = col.b + intd;
  int n = min(min(target_r, target_g), target_b);
  int x = max(max(target_r, target_g), target_b);
  int int_lum = fastround(lum * 255.0);

  if (n < 0) {
    target_r = int_lum + ((target_r - int_lum) * int_lum) / (int_lum - n);
    target_g = int_lum + ((target_g - int_lum) * int_lum) / (int_lum - n);
    target_b = int_lum + ((target_b - int_lum) * int_lum) / (int_lum - n);
  }

  if (x > 255) {
    target_r =
      int_lum + ((target_r - int_lum) * (255 - int_lum)) / (x - int_lum);
    target_g =
      int_lum + ((target_g - int_lum) * (255 - int_lum)) / (x - int_lum);
    target_b =
      int_lum + ((target_b - int_lum) * (255 - int_lum)) / (x - int_lum);
  }

  return color(col.a, target_r, target_g, target_b);
}

static int
_comp_sat(
  color_t_ col)
{
  int n = min(min(col.r, col.g), col.b);
  int x = max(max(col.r, col.g), col.b);
  return x - n;
}

static color_t_
_comp_set_sat(
  color_t_ col,
  int sat)
{
  uint8_t *c_min, *c_mid, *c_max;

  if (col.r >= col.g) {
    c_max = &col.r;
    c_mid = &col.g;
  } else {
    c_max = &col.g;
    c_mid = &col.r;
  }

  if (col.b >= *c_max) {
    c_min = c_mid;
    c_mid = c_max;
    c_max = &col.b;
  } else if (col.b >= *c_mid) {
    c_min = c_mid;
    c_mid = &col.b;
  } else {
    c_min = &col.b;
  }

  if (*c_max > *c_min) {
    *c_mid = ((*c_mid - *c_min) * sat) / (*c_max - *c_min);
    *c_max = sat;
  } else {
    *c_mid = 0;
    *c_max = 0;
  }

  *c_min = 0;

  return col;

}


color_t_
comp_hue(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blended_color =
    _comp_set_lum(_comp_set_sat(src, _comp_sat(dst)), _comp_lum(dst));
  blended_color.a = src.a;
  return comp_source_over(blended_color, dst, draw_alpha);
}

color_t_
comp_saturation(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blended_color =
    _comp_set_lum(_comp_set_sat(dst, _comp_sat(src)), _comp_lum(dst));
  blended_color.a = src.a;
  return comp_source_over(blended_color, dst, draw_alpha);
}

color_t_
comp_color(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blended_color = _comp_set_lum(src, _comp_lum(dst));
  blended_color.a = src.a;
  return comp_source_over(blended_color, dst, draw_alpha);
}

color_t_
comp_luminosity(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  color_t_ blended_color = _comp_set_lum(dst, _comp_lum(src));
  blended_color.a = src.a;
  return comp_source_over(blended_color, dst, draw_alpha);
}

color_t_
comp_one_minus_src(
  color_t_ src,
  color_t_ dst,
  int draw_alpha)
{
  return alpha_blend(draw_alpha, color_white, dst);
}

color_t_
comp_compose(
  color_t_ src,
  color_t_ dst,
  int draw_alpha,
  composite_operation_t composite_operation)
{
  switch (composite_operation) {
    case SOURCE_OVER:      return comp_source_over(src, dst, draw_alpha);
    case SOURCE_ATOP:      return comp_source_atop(src, dst, draw_alpha);
    case SOURCE_IN:        return comp_source_in(src, dst, draw_alpha);
    case SOURCE_OUT:       return comp_source_out(src,dst,draw_alpha);
    case DESTINATION_OVER: return comp_destination_over(src, dst, draw_alpha);
    case DESTINATION_ATOP: return comp_destination_atop(src, dst, draw_alpha);
    case DESTINATION_IN:   return comp_destination_in(src, dst, draw_alpha);
    case DESTINATION_OUT:  return comp_destination_out(src, dst, draw_alpha);
    case LIGHTER:          return comp_lighter(src, dst, draw_alpha);
    case MULTIPLY:         return comp_multiply(src, dst, draw_alpha);
    case COPY:             return comp_copy(src, dst, draw_alpha);
    case XOR:              return comp_xor(src, dst, draw_alpha);
    case SCREEN:           return comp_screen(src, dst, draw_alpha);
    case OVERLAY:          return comp_overlay(src, dst, draw_alpha);
    case LIGHTEN:          return comp_lighten(src, dst, draw_alpha);
    case DARKEN:           return comp_darken(src, dst, draw_alpha);
    case COLOR_BURN:       return comp_color_burn(src, dst, draw_alpha);
    case COLOR_DODGE:      return comp_color_dodge(src, dst, draw_alpha);
    case HARD_LIGHT:       return comp_hard_light(src, dst, draw_alpha);
    case SOFT_LIGHT:       return comp_soft_light(src, dst, draw_alpha);
    case DIFFERENCE:       return comp_difference(src, dst, draw_alpha);
    case EXCLUSION:        return comp_exclusion(src, dst, draw_alpha);
    case HUE:              return comp_hue(src, dst, draw_alpha);
    case SATURATION:       return comp_saturation(src, dst, draw_alpha);
    case LUMINOSITY:       return comp_luminosity(src, dst, draw_alpha);
    case COLOR:            return comp_color(src, dst, draw_alpha);
    case ONE_MINUS_SRC:    return comp_one_minus_src(src, dst, draw_alpha);
    default:
      assert(!"Invalid operation specified");
      return color_transparent_black;
  }
}

bool
comp_is_full_screen(
  composite_operation_t composite_operation)
{
  switch (composite_operation) {
  case SOURCE_OVER:      return false;
  case SOURCE_IN:        return true;
  case SOURCE_OUT:       return true;
  case SOURCE_ATOP:      return false;
  case DESTINATION_OVER: return false;
  case DESTINATION_IN:   return true;
  case DESTINATION_OUT:  return true;
  case DESTINATION_ATOP: return true;
  case LIGHTER:          return false;
  case COPY:             return true;
  case XOR:              return false;
  case MULTIPLY:         return false;
  case SCREEN:           return false;
  case OVERLAY:          return false;
  case DARKEN:           return false;
  case LIGHTEN:          return false;
  case COLOR_DODGE:      return false;
  case COLOR_BURN:       return false;
  case HARD_LIGHT:       return false;
  case SOFT_LIGHT:       return false;
  case DIFFERENCE:       return false;
  case EXCLUSION:        return false;
  case HUE:              return false;
  case COLOR:            return false;
  case LUMINOSITY:       return false;
  case SATURATION:       return false;
  case ONE_MINUS_SRC:    return true;
  default:
    assert(!"Invalid operation specified");
    return true;
  }
}
