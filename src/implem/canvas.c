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
#include <memory.h>
#include <math.h>
#include <assert.h>

#include "util.h"
#include "unicode.h"
#include "tuples.h"
#include "point.h"
#include "rect.h"
#include "color.h"
#include "list.h"
#include "window.h"
#include "target.h"
#include "surface.h"
#include "image_data.h"
#include "state.h"
#include "font_desc.h"
#include "font.h"
#include "transform.h"
#include "path.h"
#include "arc.h"
#include "polygon.h"
#include "polygonize.h"
#include "poly_render.h"
#include "backend.h"
#include "canvas_internal.h"

typedef enum canvas_type_t {
  CANVAS_OFFSCREEN = 0,
  CANVAS_FRAMED    = 1,
  CANVAS_FRAMELESS = 2
} canvas_type_t;

static canvas_t *
_canvas_create_internal(
  canvas_type_t type,
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  width = max(1, width);
  height = max(1, height);

  canvas_t *canvas = (canvas_t *)calloc(1, sizeof(canvas_t));
  if (canvas == NULL) {
    return NULL;
  }

  canvas->id = backend_next_id();
  if (canvas->id == 0) {
    goto error_id;
  }

  canvas->path = path_create(12, 18);
  if (canvas->path == NULL) {
    goto error_path;
  }

  canvas->state = state_create();
  if (canvas->state == NULL) {
    goto error_state;
  }

  canvas->state_stack = list_new((free_val_fun_t *)state_destroy);
  if (canvas->state_stack == NULL) {
    goto error_state_stack;
  }

  if (type == CANVAS_OFFSCREEN) {

    canvas->window = NULL;

    canvas->surface = surface_create(width, height);
    if (canvas->surface == NULL) {
      goto error_offscreen_surface;
    }

  } else if ((type == CANVAS_FRAMED) || (type == CANVAS_FRAMELESS)) {

    bool framed = (type == CANVAS_FRAMED);

    canvas->window = window_create(framed, title, x, y, width, height);
    if (canvas->window == NULL) {
      goto error_window;
    }

    window_set_user_data(canvas->window, (void *)canvas);

    target_t *target = window_get_target(canvas->window);
    if (target == NULL) {
      goto error_target;
    }

    canvas->surface = surface_create_onscreen(target, width, height);
    free(target);
    if (canvas->surface == NULL) {
      goto error_onscreen_surface;
    }

  }

  canvas->font = NULL;
  canvas->width = width;
  canvas->height = height;
  canvas->first_x = 0.0;
  canvas->first_y = 0.0;
  canvas->last_x = 0.0;
  canvas->last_y = 0.0;
  canvas->data = NULL;
  canvas->id = backend_next_id();

  backend_add_canvas(canvas);

  return canvas;

error_onscreen_surface:
error_target:
  window_destroy(canvas->window);
error_window:
error_offscreen_surface:
  list_delete(canvas->state_stack);
error_state_stack:
  state_destroy(canvas->state);
error_state:
  path_destroy(canvas->path);
error_path:
error_id:
  free(canvas);
  return NULL;
}

canvas_t *
canvas_create_framed(
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  assert(title != NULL);

  return _canvas_create_internal(CANVAS_FRAMED, title, x, y, width, height);
}

canvas_t *
canvas_create_frameless(
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_FRAMELESS, NULL, x, y, width, height);
}

canvas_t *
canvas_create_offscreen(
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_OFFSCREEN, NULL, 0, 0, width, height);
}

void
canvas_destroy(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);
  assert(canvas->path != NULL);

  backend_remove_canvas(canvas);

  surface_destroy(canvas->surface);

  /* Offscreen canvas do not have windows */
  if (canvas->window != NULL) {
    window_destroy(canvas->window);
  }

  if (canvas->font != NULL) {
    font_destroy(canvas->font);
  }

  path_destroy(canvas->path);
  list_delete(canvas->state_stack);
  state_destroy(canvas->state);
  free(canvas);
}



/* Visibility */

void
canvas_show(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  if (canvas->window != NULL) {
    window_show(canvas->window);
  }
}

void
canvas_hide(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  if (canvas->window != NULL) {
    window_hide(canvas->window);
  }
}



/* Configuration */

int32_t
canvas_get_id(
  const canvas_t *canvas)
{
  assert(canvas != NULL);

  return canvas->id;
}

void *
canvas_get_user_data(
  canvas_t *canvas)
{
  assert(canvas != NULL);

  return canvas->data;
}

void
canvas_set_user_data(
  canvas_t *canvas,
  void *data)
{
  assert(canvas != NULL);

  canvas->data = data;
}

pair_t(int32_t)
canvas_get_size(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  return pair(int32_t, canvas->width, canvas->height);
}

static void
_canvas_reset_state(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->path != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);

  path_reset(canvas->path);
  state_reset(canvas->state);
  list_reset(canvas->state_stack);

  canvas->first_x = canvas->first_y = 0.0;
  canvas->last_x = canvas->last_y = 0.0;
}

void
_canvas_set_size_internal(
  canvas_t *canvas,
  int32_t width,
  int32_t height)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  _canvas_reset_state(canvas);

  width = max(1, width);
  height = max(1, height);

  if (!surface_resize(canvas->surface, width, height)) {
    return;
  }

  canvas->width = width;
  canvas->height = height;

  if (canvas->window != NULL) {
// should probably perform an internal present event instead
// unless this is already requested internally
    present_data_t pd;
    memset((void *)&pd, 0, sizeof(present_data_t));
    surface_present(canvas->surface, &pd);
  }
}

void
canvas_set_size(
  canvas_t *canvas,
  int32_t width,
  int32_t height)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  width = max(1, width);
  height = max(1, height);

  if (canvas->window != NULL) {
    window_set_size(canvas->window, width, height);
  }

  _canvas_set_size_internal(canvas, width, height);
}

pair_t(int32_t)
canvas_get_position(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  if (canvas->window != NULL) {
    return window_get_position(canvas->window);
  }
  return pair(int32_t, 0, 0);
}

void
canvas_set_position(
  canvas_t *canvas,
  int32_t x,
  int32_t y)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);

  if (canvas->window != NULL) {
    window_set_position(canvas->window, x, y);
  }
}



/* State */

bool
canvas_save(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);

  state_t *s = state_copy(canvas->state);
  if (s == NULL) {
    return false;
  }
  if (!list_push(canvas->state_stack, (void *)canvas->state)) {
    state_destroy(s);
    return false;
  }
  canvas->state = s;
  return true;
}

void
canvas_restore(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);

  state_t *s = (state_t *)list_pop(canvas->state_stack);
  if (s != NULL) {
    state_destroy(canvas->state);
    canvas->state = s;
  }
}



/* Transform */

// getTransform (not compatible with IE) ? -> keep a copy

void
canvas_set_transform(
  canvas_t *canvas,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_set(canvas->state->transform, a, b, c, d, e, f);
}

void
canvas_transform(
  canvas_t *canvas,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_mul(canvas->state->transform, a, b, c, d, e, f);
}

void
canvas_translate(
  canvas_t *canvas,
  double x,
  double y)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_translate(canvas->state->transform, x, y);
}

void
canvas_scale(
  canvas_t *canvas,
  double x,
  double y)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_scale(canvas->state->transform, x, y);
}

void
canvas_shear(
  canvas_t *canvas,
  double x,
  double y)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_shear(canvas->state->transform, x, y);
}

void
canvas_rotate(
  canvas_t *canvas,
  double a)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  transform_rotate(canvas->state->transform, a);
}



/* Style / config */

uint32_t
canvas_get_line_width(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->line_width;
}

void
canvas_set_line_width(
  canvas_t *canvas,
  double line_width)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->line_width = line_width;
}

color_t_
canvas_get_fill_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->fill_color;
}

void
canvas_set_fill_color(
  canvas_t *canvas,
  color_t_ color)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->fill_color = color;
}

color_t_
canvas_get_stroke_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->stroke_color;
}

void
canvas_set_stroke_color(
  canvas_t *canvas,
  color_t_ color)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->stroke_color = color;
}

void
canvas_set_font(
  canvas_t *c,
  const char *family,
  double size,
  font_slant_t slant,
  int32_t weight)

{
  assert(c != NULL);
  assert(family != NULL);
  assert(size > 0.0);
  assert(weight >= 0);

  font_desc_set(c->state->font_desc, family, size, slant, weight);
/*
  double sx, sy;
  transform_extract_scale(c->state->transform, &sx, &sy);
  font_desc_scale(c->state->font_desc, sy);
*/
}



/* Paths */

bool
canvas_clear_path(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->path != NULL);

  path_reset(c->path);
  c->first_x = c->first_y = 0.0;
  c->last_x = c->last_y = 0.0;
  return true;
}

static void
_canvas_save_first(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);

  c->first_x = x;
  c->first_y = y;
}

static void
_canvas_save_last(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);

  c->last_x = x;
  c->last_y = y;
}

void
canvas_close_path(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->path != NULL);

  path_add_close_path(c->path);

  // Add a move to the first point before the close
  // This makes handling of primitives after close easier
  // TODO: this seems clumsy : if a transform is set before the close,
  // then we're going to move to some random point...
  // But arc_to needs the last untransformed point, which happens
  // to be the first point in the subpath when we close it
  canvas_move_to(c, c->first_x, c->first_y);
}

void
canvas_move_to(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  _canvas_save_first(c, x, y);
  _canvas_save_last(c, x, y);

  point_t p = { x, y };

  transform_apply(c->state->transform, &p);
  path_add_move_to(c->path, p.x, p.y);
}

void
canvas_line_to(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  if (path_empty(c->path)) {
      _canvas_save_first(c, x, y);
  }
  _canvas_save_last(c, x, y);

  point_t p = { x, y };

  transform_apply(c->state->transform, &p);
  path_add_line_to(c->path, p.x, p.y);
}

static void
_canvas_bezier_list(
  canvas_t *c,
  const double *values,
  int nb_bezier)
{
  assert(c != NULL);
  assert(values != NULL);
  assert(nb_bezier >= 0);

  canvas_line_to(c, values[0], values[1]);

  for (int i = 2; i < 2 + nb_bezier * 6; i += 6) {
    canvas_bezier_curve_to(c,
                           values[i+0], values[i+1],
                           values[i+2], values[i+3],
                           values[i+4], values[i+5]);
  }
}

void
canvas_arc(
  canvas_t *c,
  double x,
  double y,
  double r,
  double di,
  double df,
  bool ccw)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  double values[26];
  int nb_bezier = arc_to_bezier(values, x, y, r, r, di, df, ccw);
  _canvas_bezier_list(c, values, nb_bezier);
}

void
canvas_arc_to(
  canvas_t *c,
  double x1,
  double y1,
  double x2,
  double y2,
  double r)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  // When the path is completely empty, we just move to midpoint
  // Note that even when the path contains a single move,
  // we consider it is not empty and thus skip this case
  // Note: we could just call save_first and save_last
  if (path_empty(c->path)) {
    canvas_move_to(c, x1, y1);
    return;
  }

  double values[26];
  int nb_bezier =
    arcto_to_bezier(values, c->last_x, c->last_y, x1, y1, x2, y2, r);
  _canvas_bezier_list(c, values, nb_bezier);
}

void
canvas_quadratic_curve_to(
  canvas_t *c,
  double cpx,
  double cpy,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  if (path_empty(c->path)) {
      _canvas_save_first(c, cpx, cpy);
  }
  _canvas_save_last(c, x, y);

  point_t cp = { cpx, cpy };
  point_t p = { x, y };

  transform_apply(c->state->transform, &cp);
  transform_apply(c->state->transform, &p);

  path_add_quadratic_curve_to(c->path, cp.x, cp.y, p.x, p.y);
}

void
canvas_bezier_curve_to(
  canvas_t *c,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  if (path_empty(c->path)) {
      _canvas_save_first(c, cp1x, cp1y);
  }
  _canvas_save_last(c, x, y);

  point_t cp1 = { cp1x, cp1y };
  point_t cp2 = { cp2x, cp2y };
  point_t p = { x, y };

  transform_apply(c->state->transform, &cp1);
  transform_apply(c->state->transform, &cp2);
  transform_apply(c->state->transform, &p);

  path_add_bezier_curve_to(c->path, cp1.x, cp1.y, cp2.x, cp2.y, p.x, p.y);
}

void
canvas_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  canvas_move_to(c, x, y);
  canvas_line_to(c, x + width, y);
  canvas_line_to(c, x + width, y + height);
  canvas_line_to(c, x, y + height);
  canvas_close_path(c);
}

static void
_rotate_list(int nb_points, double *values, double r)
{
  assert(values != NULL);
  assert(nb_points > 0);

  double cos_r = cos(r);
  double sin_r = sin(r);

  for (int i = 0; i < nb_points * 2; i += 2) {
    double tx = values[i] * cos_r + values[i+1] * sin_r;
    double ty = values[i+1] * cos_r - values[i] * sin_r;
    values[i] = tx;
    values[i+1] = ty;
  }
}

void
canvas_ellipse(
  canvas_t *c,
  double x,
  double y,
  double rx,
  double ry,
  double r,
  double di,
  double df,
  bool cc)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);

  double values[26];
  int nb_bezier = arc_to_bezier(values, x, y, rx, ry, di, df, cc);
  _rotate_list(1 + nb_bezier * 3, values, -r);
  _canvas_bezier_list(c, values, nb_bezier);
}



/* Path stroking/filling */

void
canvas_fill(
  canvas_t *c,
  bool non_zero)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = rect(point(0.0, 0.0), point(c->width, c->height));

  if (polygonize(c->path, p, &bbox) == true) {
    poly_render(c->surface, p, &bbox, c->state->fill_color, non_zero);
  }

  polygon_destroy(p);
}

void
canvas_stroke(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->path != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = rect(point(0.0, 0.0), point(c->width, c->height));

  if (polygonize_outline(c->path, c->state->line_width, p, &bbox) == true) {
    poly_render(c->surface, p, &bbox, c->state->stroke_color, true);
  }

  polygon_destroy(p);

  // Thoughts: we could polygonize on the fly when building the path
  // TODO: the line width actually depends on the current transform !
}



/* Immediate drawing */

void
canvas_fill_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height)
{
  assert(c != NULL);
  assert(c->state != NULL);

  polygon_t *p = polygon_create(8, 1);
  if (p == NULL) {
    return;
  }

  point_t p1 = point(x, y);
  point_t p2 = point(x + width, y);
  point_t p3 = point(x + width, y + height);
  point_t p4 = point(x, y + height);

  transform_apply(c->state->transform, &p1);
  transform_apply(c->state->transform, &p2);
  transform_apply(c->state->transform, &p3);
  transform_apply(c->state->transform, &p4);

  polygon_add_point(p, p1);
  polygon_add_point(p, p2);
  polygon_add_point(p, p3);
  polygon_add_point(p, p4);
  polygon_end_subpoly(p, true);

  rect_t bbox = rect(point(min4(p1.x, p2.x, p3.x, p4.x),
                           min4(p1.y, p2.y, p3.y, p4.y)),
                     point(max4(p1.x, p2.x, p3.x, p4.x),
                           max4(p1.y, p2.y, p3.y, p4.y)));

  poly_render(c->surface, p, &bbox, c->state->fill_color, false);

  polygon_destroy(p);
}

void
canvas_stroke_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height)
{
  assert(c != NULL);
  assert(c->state != NULL);

  polygon_t *p = polygon_create(8, 1);
  if (p == NULL) {
    return;
  }

  point_t p1 = point(x, y);
  point_t p2 = point(x + width, y);
  point_t p3 = point(x + width, y + height);
  point_t p4 = point(x, y + height);

  transform_apply(c->state->transform, &p1);
  transform_apply(c->state->transform, &p2);
  transform_apply(c->state->transform, &p3);
  transform_apply(c->state->transform, &p4);

  polygon_add_point(p, p1);
  polygon_add_point(p, p2);
  polygon_add_point(p, p3);
  polygon_add_point(p, p4);
  polygon_end_subpoly(p, true);

  double d = c->state->line_width;

  rect_t bbox = rect(point(min4(p1.x, p2.x, p3.x, p4.x) - d,
                           min4(p1.y, p2.y, p3.y, p4.y) - d),
                     point(max4(p1.x, p2.x, p3.x, p4.x) + d,
                           max4(p1.y, p2.y, p3.y, p4.y) + d));

  polygon_t *tp = polygon_create(16, 1);
  if (p == NULL) {
    return;
  }

  polygon_offset(p, tp, c->state->line_width);
  poly_render(c->surface, tp, &bbox, c->state->stroke_color, true);

  polygon_destroy(tp);
  polygon_destroy(p);
}

static bool
_canvas_prepare_font(
  canvas_t *c)
{
  if (c->font != NULL &&
      font_matches(c->font, c->state->font_desc) == false) {
    font_destroy(c->font);
    c->font = NULL;
  }

  if (c->font == NULL) {
    c->font = font_create(c->state->font_desc);
    if (c->font == NULL) {
      return false;
    }
  }

  return true;
}

void
canvas_fill_text(
  canvas_t *c,
  const char *text, // as UTF-8
  double x,
  double y,
  double max_width)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(text != NULL);

/*
  double sx, sy;
  transform_extract_scale(c->state->transform, &sx, &sy);
  font_desc_scale(c->state->font_desc, sy);
*/

// TODO: handle both vector and bitmap fonts

  if (_canvas_prepare_font(c) == false) {
    return;
  }

  polygon_t *p = polygon_create(256, 8);
  if (p == NULL) {
    return;
  }

  rect_t bbox = rect(point(0.0, 0.0), point(c->width, c->height));

  point_t pen = { x, y };

  while (*text) {
    uint32_t chr = decode_utf8_char(&text);
    polygon_reset(p);
    if (font_char_as_poly(c->font, c->state->transform,
                          chr, &pen, p, &bbox) == true) {
      poly_render(c->surface, p, &bbox, c->state->fill_color, true);
    }
  }

  polygon_destroy(p);
}

void
canvas_stroke_text(
  canvas_t *c,
  const char *text, // as UTF-8
  double x,
  double y,
  double max_width)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(text != NULL);

  if (_canvas_prepare_font(c) == false) {
    return;
  }

  polygon_t *p = polygon_create(256, 8);
  if (p == NULL) {
    return;
  }

  rect_t bbox = rect(point(0.0, 0.0), point(c->width, c->height));

  point_t pen = { x, y };

  while (*text) {
    uint32_t chr = decode_utf8_char(&text);
    polygon_reset(p);
    if (font_char_as_poly_outline(c->font, c->state->transform,
                                  chr, c->state->line_width,
                                  &pen, p, &bbox) == true) {
      poly_render(c->surface, p, &bbox, c->state->stroke_color, true);
    }
  }

  polygon_destroy(p);
}

void
canvas_blit(
  canvas_t *dc,
  int32_t dx,
  int32_t dy,
  const canvas_t *sc,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(dc != NULL);
  assert(sc != NULL);

  surface_t *ds = dc->surface;
  const surface_t *ss = sc->surface;
  surface_blit(ds, dx, dy, ss, sx, sy, width, height);
  // TODO: new version with transformations
}

/* Direct pixel access */

color_t_
canvas_get_pixel(
  const canvas_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);

  return surface_get_pixel(c->surface, x, y);
}

void
canvas_set_pixel(
  canvas_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);

  surface_set_pixel(c->surface, x, y, color);
}

image_data_t
canvas_get_image_data(
  const canvas_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);

  return surface_get_image_data(c->surface, sx, sy, width, height);
}

void
canvas_set_image_data(
  canvas_t *c,
  int32_t dx,
  int32_t dy,
  const image_data_t *data,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);

  surface_set_image_data(c->surface, dx, dy, data, sx, sy, width, height);
}
