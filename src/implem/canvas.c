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
#include "draw_style.h"
#include "gradient.h"
#include "transform.h"
#include "path2d.h"
#include "arc.h"
#include "polygon.h"
#include "polygon_internal.h"
#include "polygonize.h"
#include "poly_render.h"
#include "draw_instr.h"
#include "backend.h"
#include "canvas_internal.h"

IMPLEMENT_OBJECT_METHODS(canvas_t, canvas, _canvas_destroy)

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

  canvas_t *canvas = canvas_alloc();
  if (canvas == NULL) {
    return NULL;
  }

  canvas->type = type;

  canvas->id = backend_next_id();
  if (canvas->id == 0) {
    goto error_id;
  }

  canvas->path_2d = path2d_create();
  if (canvas->path_2d == NULL) {
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
      // Record the width and height first, as surface_create_from_pixmap
      // steals the image data and resets the dimensions
      width = pixmap->width;
      height = pixmap->height;
      canvas->surface = surface_create_from_pixmap(pixmap);
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

    // Retain the pointer on behalf of the window, so that
    // the canvas remains alive while the window exists
    canvas_retain(canvas);
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
  canvas->clip_region = pixmap_null();
  canvas->clip_region_dirty = false;

  canvas->id = backend_next_id();

  backend_add_canvas(canvas);

  if ((type == CANVAS_FRAMED) || (type == CANVAS_FRAMELESS)) {
    // Onscreen canvas are filled with white by default
    canvas_fill_rect(canvas, 0, 0, width, height);
  }

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
  path2d_release(canvas->path_2d);
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

static void (*_canvas_destroy_callback)(canvas_t *) = NULL;

void
canvas_set_destroy_callback(
  void (*callback)(canvas_t *))
{
  _canvas_destroy_callback = callback;
}

static void
_canvas_destroy(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->surface != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);
  assert(canvas->path_2d != NULL);

  if (_canvas_destroy_callback != NULL) {
    _canvas_destroy_callback(canvas);
  }

  backend_remove_canvas(canvas);

  surface_destroy(canvas->surface);

  /* Offscreen and closed canvas do not have windows */
  if (canvas->window != NULL) {
    window_destroy(canvas->window);
  }

  if (canvas->font != NULL) {
    font_destroy(canvas->font);
  }

  if (pixmap_valid(canvas->clip_region) == true) {
    pixmap_destroy(canvas->clip_region);
  }

  path2d_release(canvas->path_2d);
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

void canvas_close(
  canvas_t *canvas)
{
  assert(canvas != NULL);

  if (canvas->window != NULL) {
    assert(canvas == (canvas_t *)window_get_data(canvas->window));
    window_destroy(canvas->window);
    canvas->window = NULL;
    // Release the pointer that was in window data, so that the canvas
    // can be freed if not referenced by any other pointer
    canvas_release(canvas);
  }
}

bool canvas_is_closed(
  const canvas_t *canvas)
{
  assert(canvas != NULL);

  return (canvas->window == NULL);
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
  assert(canvas->path_2d != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);

  state_reset(canvas->state);
  list_reset(canvas->state_stack);
  path2d_reset(canvas->path_2d);
  if (pixmap_valid(canvas->clip_region) == true) {
    pixmap_destroy(canvas->clip_region);
    canvas->clip_region_dirty = false;
  }
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
    if (pixmap_valid(canvas->clip_region) == true) {
      pixmap_destroy(canvas->clip_region);
    }
    canvas->clip_region_dirty = !list_is_empty(canvas->state->clip_path);
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

join_type_t
canvas_get_join_type(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->join_type;
}

void
canvas_set_join_type(
  canvas_t *canvas,
  join_type_t join_type)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->join_type = join_type;
}

cap_type_t
canvas_get_cap_type(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->cap_type;
}

void
canvas_set_cap_type(
  canvas_t *canvas,
  cap_type_t cap_type)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->cap_type = cap_type;
}

double
canvas_get_line_dash_offset(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->line_dash_offset;
}

void
canvas_set_line_dash_offset(
  canvas_t *canvas,
  double line_dash_offset)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->line_dash_offset = line_dash_offset;
}

const double *
canvas_get_line_dash(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->line_dash;
}

size_t
canvas_get_line_dash_length(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->line_dash_len;
}

void
canvas_set_line_dash(
  canvas_t *canvas,
  const double *dash,
  size_t n)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (canvas->state->line_dash != NULL) {
    free(canvas->state->line_dash);
  }

  if ((dash == NULL) || (n == 0)) {
    canvas->state->line_dash = NULL;
    canvas->state->line_dash_len = 0;
  } else if ((n % 2) == 0) {
    canvas->state->line_dash = (double *)memdup(dash, n * sizeof(double));
    canvas->state->line_dash_len = n;
  } else {
    double *new_dash = (double *)calloc(2 * n, sizeof(double));
    for (size_t i = 0; i < 2 * n; ++i) {
      new_dash[i] = dash[i % n];
    }
    canvas->state->line_dash = new_dash;
    canvas->state->line_dash_len = 2 * n;
  }
}

color_t_
canvas_get_stroke_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (canvas->state->stroke_style.type == DRAW_STYLE_COLOR) {
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

  draw_style_destroy(&canvas->state->stroke_style);
  canvas->state->stroke_style.type = DRAW_STYLE_COLOR;
  canvas->state->stroke_style.content.color = color;
}

void
canvas_set_stroke_gradient(
  canvas_t *canvas,
  gradient_t *gradient)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(gradient != NULL);

  draw_style_destroy(&canvas->state->stroke_style);
  canvas->state->stroke_style.type = DRAW_STYLE_GRADIENT;
  canvas->state->stroke_style.content.gradient = gradient_retain(gradient);
}

void
canvas_set_stroke_pattern(
  canvas_t *canvas,
  pattern_t *pattern)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(pattern != NULL);

  draw_style_destroy(&canvas->state->stroke_style);
  canvas->state->stroke_style.type = DRAW_STYLE_PATTERN;
  canvas->state->stroke_style.content.pattern = pattern_retain(pattern);
}

draw_style_t
canvas_get_stroke_style(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->stroke_style;
}

color_t_
canvas_get_fill_color(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (canvas->state->fill_style.type == DRAW_STYLE_COLOR) {
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

  draw_style_destroy(&canvas->state->fill_style);
  canvas->state->fill_style.type = DRAW_STYLE_COLOR;
  canvas->state->fill_style.content.color = color;
}

void
canvas_set_fill_gradient(
  canvas_t *canvas,
  gradient_t *gradient)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(gradient != NULL);

  draw_style_destroy(&canvas->state->fill_style);
  canvas->state->fill_style.type = DRAW_STYLE_GRADIENT;
  canvas->state->fill_style.content.gradient = gradient_retain(gradient);
}

void
canvas_set_fill_pattern(
  canvas_t *canvas,
  pattern_t *pattern)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(pattern != NULL);

  draw_style_destroy(&canvas->state->fill_style);
  canvas->state->fill_style.type = DRAW_STYLE_PATTERN;
  canvas->state->fill_style.content.pattern = pattern_retain(pattern);
}

draw_style_t
canvas_get_fill_style(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->fill_style;
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

composite_operation_t
canvas_get_comp_operation(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->global_composite_operation;
}

void
canvas_set_comp_operation(
  canvas_t *canvas,
  composite_operation_t op)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  canvas->state->global_composite_operation = op;
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
  assert(c->path_2d != NULL);

  path2d_reset(c->path_2d);

  return true;
}

void
canvas_close_path(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);

  path2d_close(c->path_2d);
}

void
canvas_move_to(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_move_to(c->path_2d, x, y, c->state->transform);
}

void
canvas_line_to(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_line_to(c->path_2d, x, y, c->state->transform);
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
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_arc(c->path_2d, x, y, r, di, df, ccw, c->state->transform);
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
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_arc_to(c->path_2d, x1, y1, x2, y2, r, c->state->transform);
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
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_quadratic_curve_to(c->path_2d, cpx, cpy, x, y, c->state->transform);
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
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_bezier_curve_to(c->path_2d, cp1x, cp1y, cp2x, cp2y, x, y,
                         c->state->transform);
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
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_rect(c->path_2d, x, y, width, height, c->state->transform);
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
  bool ccw)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);
  assert(c->state != NULL);

  path2d_ellipse(c->path_2d, x, y, rx, ry, r, di, df, ccw, c->state->transform);
}



/* Path stroking/filling */


static void
_canvas_clip_fill_instr(
  canvas_t *c,
  const path_fill_instr_t *instr)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(pixmap_valid(c->clip_region) == true);
  assert(instr != NULL);
  assert(instr->poly != NULL);

  draw_style_t white = (draw_style_t){ .type = DRAW_STYLE_COLOR,
                                       .content.color = color_white };
  rect_t bbox = rect(point(0.0, 0.0),
                     point((double)c->width, (double)c->height));
  poly_render(&(c->clip_region), instr->poly, &bbox,
              white, 1.0, ONE_MINUS_SRC,
              NULL, instr->non_zero, c->state->transform);
}

static bool
_canvas_clip_region_ensure(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(c->state->clip_path != NULL);

  if (c->clip_region_dirty == false) {
    return true;
  }

  if (pixmap_valid(c->clip_region) == false) {
    c->clip_region = pixmap(c->width, c->height, NULL);
    if (pixmap_valid(c->clip_region) == false) {
      return false;
    }
  }

  for (int32_t j = 0; j < c->height; ++j) {
    for (int32_t i = 0; i < c->width; ++i) {
      pixmap_at(c->clip_region, j, i) = color_transparent_black;
    }
  }

  list_iterator_t *it = list_get_iterator(c->state->clip_path);
  if (it == NULL) {
    return false;
  }

  path_fill_instr_t *instr = NULL;
  while ((instr = (path_fill_instr_t *)list_iterator_next(it)) != NULL) {
    _canvas_clip_fill_instr(c, instr);
  }

  list_free_iterator(it);

  c->clip_region_dirty = false;

  return true;
}


void
canvas_fill(
  canvas_t *c,
  bool non_zero)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);

  _canvas_clip_region_ensure(c);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize(path2d_get_path(c->path_2d), p, &bbox) == true) {
    pixmap_t pm = surface_get_raw_pixmap(c->surface);
    poly_render(&pm, p, &bbox,
                c->state->fill_style, c->state->global_alpha,
                c->state->global_composite_operation,
                &(c->clip_region), non_zero, c->state->transform);
  }

  polygon_destroy(p);
}

void
canvas_fill_path(
  canvas_t *c,
  path2d_t *path,
  bool non_zero)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);
  assert(path != NULL);

  _canvas_clip_region_ensure(c);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize(path2d_get_path(path), p, &bbox) == true) {

    // Apply transformation
    for (int i = 0; i < p->nb_points; ++i) {
      transform_apply(c->state->transform, &(p->points[i]));
    }

    // Update bbox
    point_t pt1 = transform_apply_new(c->state->transform, &bbox.p1);
    point_t pt2 = transform_apply_new(c->state->transform, &bbox.p2);
    point_t bp3 = point(bbox.p2.x, bbox.p1.y);
    point_t bp4 = point(bbox.p1.x, bbox.p2.y);
    point_t pt3 = transform_apply_new(c->state->transform, &bp3);
    point_t pt4 = transform_apply_new(c->state->transform, &bp4);
    double xmin = min(pt1.x, min(pt2.x, min(pt3.x, pt4.x)));
    double ymin = min(pt1.y, min(pt2.y, min(pt3.y, pt4.y)));
    double xmax = max(pt1.x, max(pt2.x, max(pt3.x, pt4.x)));
    double ymax = max(pt1.y, max(pt2.y, max(pt3.y, pt4.y)));
    bbox.p1 = point(xmin, ymin);
    bbox.p2 = point(xmax, ymax);

    pixmap_t pm = surface_get_raw_pixmap(c->surface);
    poly_render(&pm, p, &bbox,
                c->state->fill_style, c->state->global_alpha,
                c->state->global_composite_operation,
                &(c->clip_region), non_zero, c->state->transform);
  }

  polygon_destroy(p);
}

void
canvas_stroke(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);
  assert(c->path_2d != NULL);

  _canvas_clip_region_ensure(c);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize_outline(path2d_get_path(c->path_2d),
                         c->state->line_width, p, &bbox,
                         c->state->join_type, c->state->cap_type,
                         c->state->transform, true,
                         c->state->line_dash, c->state->line_dash_len,
                         c->state->line_dash_offset) == true) {
    pixmap_t pm = surface_get_raw_pixmap(c->surface);
    poly_render(&pm, p, &bbox,
                c->state->stroke_style, c->state->global_alpha,
                c->state->global_composite_operation,
                &(c->clip_region), true, c->state->transform);
  }

  polygon_destroy(p);
}

void
canvas_stroke_path(
  canvas_t *c,
  path2d_t *path)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(c->surface != NULL);
  assert(path != NULL);

  _canvas_clip_region_ensure(c);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize_outline(path2d_get_path(path),
                         c->state->line_width, p, &bbox,
                         c->state->join_type, c->state->cap_type,
                         c->state->transform, false,
                         c->state->line_dash, c->state->line_dash_len,
                         c->state->line_dash_offset) == true) {
    pixmap_t pm = surface_get_raw_pixmap(c->surface);
    poly_render(&pm, p, &bbox,
                c->state->stroke_style, c->state->global_alpha,
                c->state->global_composite_operation,
                &(c->clip_region), true, c->state->transform);
  }

  polygon_destroy(p);
}

void
canvas_clip(
  canvas_t *c,
  bool non_zero)
{
  assert(c != NULL);
  assert(c->path_2d != NULL);
  assert(c->state != NULL);
  assert(c->state->clip_path != NULL);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize(path2d_get_path(c->path_2d), p, &bbox) == true) {
    list_push(c->state->clip_path, path_fill_instr_create(p, non_zero));
  }

  polygon_destroy(p);

  c->clip_region_dirty = true;
}

void
canvas_clip_path(
  canvas_t *c,
  path2d_t *path,
  bool non_zero)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(path != NULL);

  // TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize(path2d_get_path(path), p, &bbox) == true) {
    for (int32_t i = 0; i < p->nb_points; ++i) {
      transform_apply(c->state->transform, &(p->points[i]));
    }
    list_push(c->state->clip_path, path_fill_instr_create(p, non_zero));
  }

  polygon_destroy(p);

  c->clip_region_dirty = true;
}


/* Immediate drawing */

static polygon_t *
_canvas_build_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height,
  rect_t *bbox) // out
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(bbox != NULL);

  polygon_t *p = polygon_create(8, 1);
  if (p == NULL) {
    return NULL;
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

  *bbox = rect(point(min4(p1.x, p2.x, p3.x, p4.x),
                     min4(p1.y, p2.y, p3.y, p4.y)),
               point(max4(p1.x, p2.x, p3.x, p4.x),
                     max4(p1.y, p2.y, p3.y, p4.y)));

  return p;
}

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

  _canvas_clip_region_ensure(c);

  rect_t bbox = { 0 };
  polygon_t *p = _canvas_build_rect(c, x, y, width, height, &bbox);
  if (p == NULL) {
    return;
  }

  pixmap_t pm = surface_get_raw_pixmap(c->surface);
  poly_render(&pm, p, &bbox,
              c->state->fill_style, c->state->global_alpha,
              c->state->global_composite_operation,
              &(c->clip_region), false, c->state->transform);

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

  _canvas_clip_region_ensure(c);

  rect_t bbox = { 0 };
  polygon_t *p = _canvas_build_rect(c, x, y, width, height, &bbox);
  if (p == NULL) {
    return;
  }

  double d = c->state->line_width;
  bbox.p1.x -= d; bbox.p1.y -= d;
  bbox.p2.x += d; bbox.p2.y += d;

  polygon_t *tp = polygon_create(16, 1);
  if (p == NULL) {
    polygon_destroy(p);
    return;
  }

  polygon_offset(p, tp, c->state->line_width, JOIN_ROUND, CAP_BUTT,
                 c->state->transform, true, c->state->line_dash,
                 c->state->line_dash_len, c->state->line_dash_offset);

  pixmap_t pm = surface_get_raw_pixmap(c->surface);
  poly_render(&pm, tp, &bbox,
              c->state->stroke_style, c->state->global_alpha,
              c->state->global_composite_operation,
              &(c->clip_region), true, c->state->transform);

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

  _canvas_clip_region_ensure(c);

  if (_canvas_prepare_font(c) == false) {
    return;
  }

  polygon_t *p = polygon_create(256, 8);
  if (p == NULL) {
    return;
  }

  point_t pen = { x, y };
  while (*text) {
    polygon_reset(p);
    uint32_t chr = decode_utf8_char(&text);
    rect_t bbox = { 0 };
    if (font_char_as_poly(c->font, c->state->transform,
                          chr, &pen, p, &bbox) == true) {
      pixmap_t pm = surface_get_raw_pixmap(c->surface);
      poly_render(&pm, p, &bbox,
                  c->state->fill_style, c->state->global_alpha,
                  c->state->global_composite_operation,
                  &(c->clip_region), true, c->state->transform);
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

  _canvas_clip_region_ensure(c);

  if (_canvas_prepare_font(c) == false) {
    return;
  }

  polygon_t *p = polygon_create(256, 8);
  if (p == NULL) {
    return;
  }

  point_t pen = { x, y };
  while (*text) {
    polygon_reset(p);
    uint32_t chr = decode_utf8_char(&text);
    rect_t bbox = { 0 };
    if (font_char_as_poly_outline(c->font, c->state->transform,
                                  chr, c->state->line_width,
                                  &pen, p, &bbox) == true) {
      pixmap_t pm = surface_get_raw_pixmap(c->surface);
      poly_render(&pm, p, &bbox,
                  c->state->stroke_style, c->state->global_alpha,
                  c->state->global_composite_operation,
                  &(c->clip_region), true, c->state->transform);
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
    transform_extract_translation(dc->state->transform, &tx, &ty);
    int32_t lo_x = max(dx + (int32_t)tx, 0);
    int32_t hi_x = min(dx + (int32_t)tx + width, dc->width);
    int32_t lo_y = max(dy + (int32_t)ty, 0);
    int32_t hi_y = min(dy + (int32_t)ty + height, dc->height);
    for (int32_t i = lo_x; i < hi_x; i++) {
      for (int32_t j = lo_y; j < hi_y; j++) {
        int32_t uvx = i + sx - dx - (int32_t)tx;
        int32_t uvy = j + sy - dy - (int32_t)ty;
        if (uvx < 0 || uvx >= sc->width ||
            uvy < 0 || uvy >= sc->height) {
              continue;
        }
        color_t_ fill_color = pixmap_at(sp, uvy, uvx);
        int draw_alpha = fill_color.a;
        if (pixmap_valid(dc->clip_region) == true) {
          draw_alpha *= 255 - pixmap_at(dc->clip_region, j, i).a;
          draw_alpha /= 255;
        }
        color_t_ compose_result =
          comp_compose(fill_color, pixmap_at(dp, j, i), draw_alpha,
                       dc->state->global_composite_operation);
        pixmap_at(dp, j, i) = compose_result;
      }
    }
  } else {
    // Calculate output mesh
    point_t p1 = point((double)dx, (double)dy);
    point_t p2 = point((double)(dx + width), (double)dy);
    point_t p3 = point((double)(dx + width), (double)(dy + height));
    point_t p4 = point((double)dx, (double)(dy + height));

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
        point_t p = point((double)x, (double)y);
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
        int draw_alpha = fill_color.a;
        if (pixmap_valid(dc->clip_region) == true) {
          draw_alpha *= 255 - pixmap_at(dc->clip_region, y, x).a;
          draw_alpha /= 255;
        }
        color_t_ compose_result =
          comp_compose(fill_color, pixmap_at(dp, y, x), draw_alpha,
                       dc->state->global_composite_operation);
        pixmap_at(dp, y, x) = compose_result;
      }
    }

    transform_destroy(inv_transform);
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
