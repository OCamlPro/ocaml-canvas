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
#include "list.h"
#include "color.h"
#include "transform.h"
#include "font_desc.h"
#include "draw_style.h"
#include "polygonize.h"
#include "color_composition.h"
#include "draw_instr.h"
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

  s->line_dash = NULL;

  s->clip_path = list_new((free_val_fun_t*)path_fill_instr_destroy);
  if (s->clip_path == NULL) {
    transform_destroy(s->transform);
    font_desc_destroy(s->font_desc);
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
  assert(s->transform != NULL);
  assert(s->font_desc != NULL);
  assert(s->clip_path != NULL);

  draw_style_destroy(&s->fill_style);
  draw_style_destroy(&s->stroke_style);
  if (s->line_dash != NULL) {
    free(s->line_dash);
  }
  list_delete(s->clip_path);
  font_desc_destroy(s->font_desc);
  transform_destroy(s->transform);

  free(s);
}

void
state_reset(
  state_t *s)
{
  assert(s != NULL);
  assert(s->transform != NULL);
  assert(s->font_desc != NULL);
  assert(s->clip_path != NULL);

  transform_reset(s->transform);
  font_desc_reset(s->font_desc);
  list_reset(s->clip_path);

  if (s->line_dash != NULL) {
    free(s->line_dash);
  }

  draw_style_destroy(&s->fill_style);
  s->fill_style.type = DRAW_STYLE_COLOR;
  s->fill_style.content.color = color_white;

  draw_style_destroy(&s->stroke_style);
  s->stroke_style.type = DRAW_STYLE_COLOR;
  s->stroke_style.content.color = color_black;

  s->line_dash = NULL;
  s->line_dash_len = 0;
  s->line_dash_offset = 0;
  s->line_width = 1.0;
  s->global_alpha = 1.0;
  s->shadow_blur = 0;
  s->shadow_color = color_transparent_black;
  s->shadow_offset_x = 0;
  s->shadow_offset_y = 0;
  s->miter_limit = 10.0;
  s->join_type = JOIN_ROUND;
  s->cap_type = CAP_BUTT;
  s->global_composite_operation = SOURCE_OVER;
}

state_t *
state_copy(
  const state_t *s)
{
  assert(s != NULL);
  assert(s->transform != NULL);
  assert(s->font_desc != NULL);
  assert(s->clip_path != NULL);

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

  sc->clip_path = list_new((free_val_fun_t*)path_fill_instr_destroy);
  if (sc->clip_path == NULL) {
    transform_destroy(sc->transform);
    font_desc_destroy(sc->font_desc);
    free(sc);
    return NULL;
  }

  if (s->line_dash) {
    sc->line_dash =
      (double *)memdup(sc->line_dash, s->line_dash_len * sizeof(double));
    if (sc->line_dash == NULL) {
      transform_destroy(sc->transform);
      font_desc_destroy(sc->font_desc);
      list_delete(s->clip_path);
      free(sc);
    }
  }

  list_iterator_t *it = list_get_iterator(s->clip_path);
  if (it == NULL) {
    transform_destroy(sc->transform);
    font_desc_destroy(sc->font_desc);
    list_delete(s->clip_path);
    if (sc->line_dash != NULL) {
      free(sc->line_dash);
    }
    free(sc);
    return NULL;
  }

  path_fill_instr_t *instr = NULL;
  while ((instr = (path_fill_instr_t *)list_iterator_next(it)) != NULL) {
    path_fill_instr_t *copy =
      path_fill_instr_create(instr->poly, instr->non_zero);
    list_push(sc->clip_path, copy);
  }
  list_free_iterator(it);

  sc->fill_style = draw_style_copy(&s->fill_style);
  sc->stroke_style = draw_style_copy(&s->stroke_style);

  sc->line_dash_len = s->line_dash_len;
  sc->line_dash_offset = s->line_dash_offset;
  sc->line_width = s->line_width;
  sc->global_alpha = s->global_alpha;
  sc->shadow_blur = s->shadow_blur;
  sc->shadow_color = s->shadow_color;
  sc->shadow_offset_x = s->shadow_offset_x;
  sc->shadow_offset_y = s->shadow_offset_y;
  sc->miter_limit = s->miter_limit;
  sc->join_type = s->join_type;
  sc->cap_type = s->cap_type;
  sc->global_composite_operation = s->global_composite_operation;
  return sc;
}
