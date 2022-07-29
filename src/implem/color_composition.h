/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __COLOR_COMPOSITION_H
#define __COLOR_COMPOSITION_H

#include <stdbool.h>

#include "color.h"

typedef enum composite_operation_t {
  SOURCE_OVER = 0,
  SOURCE_IN = 1,
  SOURCE_OUT = 2,
  SOURCE_ATOP = 3,
  DESTINATION_OVER = 4,
  DESTINATION_IN = 5,
  DESTINATION_OUT = 6,
  DESTINATION_ATOP = 7,
  LIGHTER = 8,
  COPY = 9,
  XOR = 10,
  MULTIPLY = 11,
  SCREEN = 12,
  OVERLAY = 13,
  DARKEN = 14,
  LIGHTEN = 15,
  COLOR_DODGE = 16,
  COLOR_BURN = 17,
  HARD_LIGHT = 18,
  SOFT_LIGHT = 19,
  DIFFERENCE = 20,
  EXCLUSION = 21,
  HUE = 22,
  SATURATION = 23,
  COLOR = 24,
  LUMINOSITY = 25,
  ONE_MINUS_SRC = 100, //for internal use only
} composite_operation_t;

color_t_ comp_source_over(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_source_in(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_source_out(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_source_atop(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_destination_over(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_destination_in(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_destination_out(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_destination_atop(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_lighter(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_copy(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_xor(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_multiply(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_screen(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_overlay(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_darken(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_lighten(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_color_dodge(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_color_burn(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_hard_light(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_soft_light(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_difference(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_exclusion(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_hue(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_saturation(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_color(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_luminosity(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_one_minus_src(
  color_t_ src,
  color_t_ dst,
  int draw_alpha);

color_t_ comp_compose(
  color_t_ src,
  color_t_ dst,
  int draw_alpha,
  composite_operation_t composite_operation
);

bool comp_is_full_screen(
  composite_operation_t comp);

#endif /* __COLOR_COMPOSITION_H */
