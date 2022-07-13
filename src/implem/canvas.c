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
#include "pixmap.h"
#include "impexp.h"
#include "state.h"
#include "font_desc.h"
#include "font.h"
#include "fill_style.h"
#include "gradient.h"
#include "transform.h"
#include "path.h"
#include "arc.h"
#include "polygon.h"
#include "polygonize.h"
#include "poly_render.h"
#include "backend.h"
#include "canvas_internal.h"

static canvas_t *
_canvas_create_internal(
  canvas_type_t type,
  const char *title, // as UTF-8
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height,
  pixmap_t *pixmap)
{
  width = max(1, width);
  height = max(1, height);

  canvas_t *canvas = (canvas_t *)calloc(1, sizeof(canvas_t));
  if (canvas == NULL) {
    return NULL;
  }

  canvas->type = type;

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

    if (pixmap == NULL) {
      canvas->surface = surface_create(width, height);
    } else {
      canvas->surface = surface_create_from_pixmap(pixmap);
      width = pixmap->width;
      height = pixmap->height;
    }
    if (canvas->surface == NULL) {
      goto error_offscreen_surface;
    }

  } else if ((type == CANVAS_FRAMED) || (type == CANVAS_FRAMELESS)) {

    bool framed = (type == CANVAS_FRAMED);

    canvas->window = window_create(framed, title, x, y, width, height);
    if (canvas->window == NULL) {
      goto error_window;
    }

    window_set_data(canvas->window, (void *)canvas);

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
  const char *title, // as UTF-8
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  assert(title != NULL);

  return _canvas_create_internal(CANVAS_FRAMED, title,
                                 x, y, width, height, NULL);
}

canvas_t *
canvas_create_frameless(
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_FRAMELESS, NULL,
                                 x, y, width, height, NULL);
}

canvas_t *
canvas_create_offscreen(
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_OFFSCREEN, NULL,
                                 0, 0, width, height, NULL);
}

canvas_t *
canvas_create_offscreen_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  return _canvas_create_internal(CANVAS_OFFSCREEN, NULL,
                                 0, 0, pixmap->width, pixmap->height, pixmap);
}

canvas_t *
canvas_create_offscreen_from_png(
  const char *filename) // as UTF-8
{
  assert(filename != NULL);

  pixmap_t pixmap = { 0 };
  bool res = impexp_import_png(&pixmap, 0, 0, filename);
  if (res == false) {
    return NULL;
  }
  return _canvas_create_internal(CANVAS_OFFSCREEN, NULL,
                                 0, 0, pixmap.width, pixmap.height, &pixmap);
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

canvas_type_t
canvas_get_type(
  const canvas_t *canvas)
{
  assert(canvas != NULL);

  return canvas->type;
}

int32_t
canvas_get_id(
  const canvas_t *canvas)
{
  assert(canvas != NULL);

  return canvas->id;
}

void *
canvas_get_data(
  canvas_t *canvas)
{
  assert(canvas != NULL);

  return canvas->data;
}

void
canvas_set_data(
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
canvas_get_stroke_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (canvas->state->stroke_style.fill_type == FILL_TYPE_COLOR) {
    return canvas->state->stroke_style.content.color;
  } else {
    return color_transparent_black;
  }
}

void
canvas_set_stroke_color(
  canvas_t *canvas,
  color_t_ color)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  fill_style_destroy(&canvas->state->stroke_style);
  fill_style_t style;
  style.fill_type = FILL_TYPE_COLOR;
  style.content.color = color;
  canvas->state->stroke_style = style;
}

fill_style_t
canvas_get_stroke_style(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->stroke_style;
}

void
canvas_set_stroke_gradient(
  canvas_t *canvas,
  gradient_t *gradient)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(gradient != NULL);

  fill_style_destroy(&canvas->state->stroke_style);
  fill_style_t style;
  style.fill_type = FILL_TYPE_GRADIENT;
  style.content.gradient = gradient_retain(gradient);
  canvas->state->stroke_style = style;
}

color_t_
canvas_get_fill_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (canvas->state->fill_style.fill_type == FILL_TYPE_COLOR) {
    return canvas->state->fill_style.content.color;
  } else {
    return color_transparent_black;
  }
}

void
canvas_set_fill_color(
  canvas_t *canvas,
  color_t_ color)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  fill_style_destroy(&canvas->state->fill_style);
  fill_style_t style;
  style.fill_type = FILL_TYPE_COLOR;
  style.content.color = color;
  canvas->state->fill_style = style;
}

fill_style_t
canvas_get_fill_style(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->fill_style;
}

void
canvas_set_fill_gradient(
  canvas_t *canvas,
  gradient_t *gradient)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(gradient != NULL);

  fill_style_destroy(&canvas->state->fill_style);
  fill_style_t style;
  style.fill_type = FILL_TYPE_GRADIENT;
  style.content.gradient = gradient_retain(gradient);
  canvas->state->fill_style = style;
}

double
canvas_get_global_alpha(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->global_alpha;
}

void
canvas_set_global_alpha(
  canvas_t *canvas,
  double global_alpha)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if ((global_alpha >= 0.0) && (global_alpha <= 1.0)) {
    canvas->state->global_alpha = global_alpha;
  }
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
    poly_render(c->surface, p, &bbox, c->state->fill_style,
                c->state->global_alpha, non_zero, c->state->transform);
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
    poly_render(c->surface, p, &bbox, c->state->stroke_style,
                c->state->global_alpha, true, c->state->transform);
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

  poly_render(c->surface, p, &bbox, c->state->fill_style,
              c->state->global_alpha, false, c->state->transform);

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
  poly_render(c->surface, tp, &bbox, c->state->stroke_style,
              c->state->global_alpha, true, c->state->transform);

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
      poly_render(c->surface, p, &bbox, c->state->fill_style,
                  c->state->global_alpha, true, c->state->transform);
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
      poly_render(c->surface, p, &bbox, c->state->stroke_style,
                  c->state->global_alpha, true, c->state->transform);
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
  assert(dc->surface != NULL);
  assert(sc != NULL);
  assert(sc->surface != NULL);

  pixmap_t dp = surface_get_raw_pixmap(dc->surface);
  const pixmap_t sp = surface_get_raw_pixmap((surface_t *)sc->surface);

  if (transform_is_pure_translation(dc->state->transform) == true) {
    double tx = 0.0, ty = 0.0;
    transform_get_translation(dc->state->transform, &tx, &ty);
    pixmap_blit(&dp, dx + tx, dy + ty, &sp, sx, sy, width, height);
  }
  else
  {
    // Calculate output mesh
    point_t p1 = point(dx, dy);
    point_t p2 = point(dx + width, dy);
    point_t p3 = point(dx + width, dy + height);
    point_t p4 = point(dx, dy + height);

    transform_apply(dc->state->transform, &p1);
    transform_apply(dc->state->transform, &p2);
    transform_apply(dc->state->transform, &p3);
    transform_apply(dc->state->transform, &p4);

    // Prepare transform
    transform_t *inv_transform = transform_copy(dc->state->transform);
    transform_inverse(inv_transform);

    // Calculate AABB
    int32_t aabb_min_x = min4(p1.x, p2.x, p3.x, p4.x);
    int32_t aabb_min_y = min4(p1.y, p2.y, p3.y, p4.y);
    int32_t aabb_max_x = max4(p1.x, p2.x, p3.x, p4.x);
    int32_t aabb_max_y = max4(p1.y, p2.y, p3.y, p4.y);

    // Correct AABBs
    aabb_min_x = max(0, min(dc->width - 1, aabb_min_x));
    aabb_max_x = max(0, min(dc->width - 1, aabb_max_x));
    aabb_min_y = max(0, min(dc->height - 1, aabb_min_y));
    aabb_max_y = max(0, min(dc->height - 1, aabb_max_y));

    // Run through
    for (int32_t x = aabb_min_x; x <= aabb_max_x; x++) {
      for (int32_t y = aabb_min_y; y <= aabb_max_y; y++) {
        point_t p = point(x, y);
        transform_apply(inv_transform, &p);
        // Get coords
        double uvx = p.x + sx - dx;
        double uvy = p.y + sy - dy;
        if ((uvx <= -1.0) || (uvx >= (double)width) ||
            (uvy <= -1.0) || (uvy >= (double)height)) {
          continue;
        }
        int32_t pt_x = (int32_t)uvx;
        int32_t pt_y = (int32_t)uvy;
        double dec_x = uvx - (double)pt_x;
        double dec_y = uvy - (double)pt_y;
        color_t_ col11 =
          pixmap_at(sp, pt_y, pt_x);
        color_t_ col21 =
          (pt_x + 1 < width) ?
          pixmap_at(sp, pt_y, pt_x + 1) :
          color_transparent_black;
        color_t_ col12 =
          (pt_y + 1 < height) ?
          pixmap_at(sp, pt_y + 1, pt_x) :
          color_transparent_black;
        color_t_ col22 =
          ((pt_x + 1 < width) && (pt_y + 1 < height)) ?
          pixmap_at(sp, pt_y + 1, pt_x + 1) :
          color_transparent_black;
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
        color_t_ fill_color = color(a, r, g, b);
        color_t_ final_color = alpha_blend(a, pixmap_at(dp, y, x), fill_color);
        pixmap_at(dp, y, x) = final_color;
      }
    }
  }
}

/* Direct pixel access */

color_t_
canvas_get_pixel(
  const canvas_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);
  assert(c->surface != NULL);

  color_t_ color = color_black;

  const pixmap_t pm = surface_get_raw_pixmap((surface_t *)c->surface);
  if (pixmap_valid(pm) == true) {
    if ((x >= 0) && (x < pm.width) && (y >= 0) && (y < pm.height)) {
      color = pixmap_at(pm, y, x);
    }
  }

  return color;
}

void
canvas_set_pixel(
  canvas_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);
  assert(c->surface != NULL);

  pixmap_t pm = surface_get_raw_pixmap(c->surface);
  if (pixmap_valid(pm) == true) {
    if ((x >= 0) && (x < pm.width) && (y >= 0) && (y < pm.height)) {
      pixmap_at(pm, y, x) = color;
    }
  }
}

pixmap_t
canvas_get_pixmap(
  const canvas_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->surface != NULL);

  pixmap_t dp = { 0 };

  const pixmap_t sp = surface_get_raw_pixmap((surface_t *)c->surface);
  if (pixmap_valid(sp) == true) {
    dp = pixmap(width, height, NULL);
    if (pixmap_valid(dp) == true) {
      pixmap_blit(&dp, 0, 0, &sp, sx, sy, width, height);
    }
  }

  return dp;
}

void
canvas_set_pixmap(
  canvas_t *c,
  int32_t dx,
  int32_t dy,
  const pixmap_t *sp,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->surface != NULL);
  assert(sp != NULL);
  assert(pixmap_valid(*sp) == true);

  pixmap_t dp = surface_get_raw_pixmap(c->surface);
  if (pixmap_valid(dp) == true) {
    pixmap_blit(&dp, dx, dy, sp, sx, sy, width, height);
  }
}

/* Import / export functions */

bool
canvas_export_png(
  const canvas_t *c,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(c->surface != NULL);
  assert(filename != NULL);

  const pixmap_t pm = surface_get_raw_pixmap((surface_t *)c->surface);
  if (pixmap_valid(pm) == false) {
    return false;
  }
  return impexp_export_png(&pm, filename);
}

bool
canvas_import_png(
  canvas_t *c,
  int32_t x,
  int32_t y,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(c->surface != NULL);
  assert(filename != NULL);

  pixmap_t pm = surface_get_raw_pixmap(c->surface);
  if (pixmap_valid(pm) == false) {
    return false;
  }
  return impexp_import_png(&pm, x, y, filename);
}
