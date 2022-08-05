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

#include "util.h"
#include "color.h"
#include "transform.h"
#include "font_desc.h"
#include "draw_style.h"
#include "polygonize.h"
#include "color_composition.h"
#include "state.h"

state_t *
state_create(
  void)
{
  state_t *s = (state_t *)calloc(1, sizeof(state_t));
  if (s == NULL) {
    return NULL;
  }

  s->transform = transform_create();
  if (s->transform == NULL) {
    free(s);
    return NULL;
  }

  s->font_desc = font_desc_create();
  if (s->font_desc == NULL) {
    transform_destroy(s->transform);
    free(s);
    return NULL;
  }

  s->fill_style.type = DRAW_STYLE_COLOR;
  s->stroke_style.type = DRAW_STYLE_COLOR;

  state_reset(s);

  return s;
}

void
state_destroy(
  state_t *s)
{
  assert(s != NULL);
  assert(s->font_desc != NULL);
  assert(s->transform != NULL);

  draw_style_destroy(&s->fill_style);
  draw_style_destroy(&s->stroke_style);
  font_desc_destroy(s->font_desc);
  transform_destroy(s->transform);
  if (s->line_dash != NULL) {
    free(s->line_dash);
  }

  free(s);
}

void
state_reset(
  state_t *s)
{
  assert(s != NULL);
  assert(s->transform != NULL);
  assert(s->font_desc != NULL);

  transform_reset(s->transform);
  font_desc_reset(s->font_desc);
  draw_style_destroy(&s->fill_style);
  s->fill_style.type = DRAW_STYLE_COLOR;
  s->fill_style.content.color = color_white;
  draw_style_destroy(&s->stroke_style);
  s->stroke_style.type = DRAW_STYLE_COLOR;
  s->stroke_style.content.color = color_black;
  s->global_alpha = 1.0;
  s->line_width = 1.0;
  s->join_type = JOIN_ROUND;
  s->cap_type = CAP_BUTT;
  s->global_composite_operation = SOURCE_OVER;
  if (s->line_dash != NULL) {
    free(s->line_dash);
  }
  s->line_dash = NULL;
  s->line_dash_len = 0;
  s->line_dash_offset = 0;
}

state_t *
state_copy(
  const state_t *s)
{
  assert(s != NULL);
  assert(s->transform != NULL);
  assert(s->font_desc != NULL);

  state_t *sc = (state_t *)calloc(1, sizeof(state_t));
  if (sc == NULL) {
    return NULL;
  }

  sc->transform = transform_copy(s->transform);
  if (sc->transform == NULL) {
    free(sc);
    return NULL;
  }

  sc->font_desc = font_desc_copy(s->font_desc);
  if (sc->font_desc == NULL) {
    transform_destroy(sc->transform);
    free(sc);
    return NULL;
  }
  sc->fill_style = draw_style_copy(&s->fill_style);
  sc->stroke_style = draw_style_copy(&s->stroke_style);
  sc->global_alpha = s->global_alpha;
  sc->line_width = s->line_width;
  sc->join_type = s->join_type;
  sc->cap_type = s->cap_type;
  sc->global_composite_operation = s->global_composite_operation;
  sc->line_dash_offset = s->line_dash_offset;
  if (s->line_dash) {
    sc->line_dash =
      (double *)memdup(sc->line_dash, s->line_dash_len * sizeof(double));
  }
  sc->line_dash_len = s->line_dash_len;

  return sc;
}
