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
#include <string.h>
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
#include "context.h"
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
#include "image_interpolation.h"
#include "filters.h"
#include "backend.h"
#include "canvas_internal.h"

IMPLEMENT_OBJECT_METHODS(canvas_t, canvas, _canvas_destroy)

static canvas_t *
_canvas_create_internal(
  canvas_type_t type,
  bool autocommit,    /* on-screen only */
  bool decorated,     /* on-screen only */
  bool resizeable,    /* on-screen only */
  bool minimize,      /* on-screen only */
  bool maximize,      /* on-screen only */
  bool close,         /* on-screen only */
  const char *title,  /* on-screen only */ // as UTF-8
  int32_t x,          /* on-screen only */
  int32_t y,          /* on-screen only */
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
      canvas->context = context_create(width, height);
    } else {
      // Record the width and height first, as context_create_from_pixmap
      // steals the image data and resets the dimensions
      width = pixmap->width;
      height = pixmap->height;
      canvas->context = context_create_from_pixmap(pixmap);
    }
    if (canvas->context == NULL) {
      goto error_offscreen_context;
    }

  } else if (type == CANVAS_ONSCREEN) {

    canvas->window = window_create(decorated, resizeable, minimize, maximize,
                                   close, title, x, y, width, height);
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

    canvas->context = context_create_onscreen(target, width, height);
    free(target);
    if (canvas->context == NULL) {
      goto error_onscreen_context;
    }
  }

  canvas->font = NULL;
  canvas->width = width;
  canvas->height = height;
  canvas->clip_region_dirty = false;

  canvas->autocommit = autocommit;
  canvas->committed = false;

  canvas->id = backend_next_id();

  backend_add_canvas(canvas);

  if (type == CANVAS_ONSCREEN) {
    // Onscreen canvas are filled with white by default
    canvas_fill_rect(canvas, 0, 0, width, height);
  }

  return canvas;

error_onscreen_context:
error_target:
  window_destroy(canvas->window);
error_window:
error_offscreen_context:
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
canvas_create_onscreen(
  bool autocommit,
  bool decorated,
  bool resizeable,
  bool minimize,
  bool maximize,
  bool close,
  const char *title, // as UTF-8
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_ONSCREEN, autocommit,
                                 decorated, resizeable,
                                 minimize, maximize, close,
                                 title, x, y, width, height, NULL);
}

canvas_t *
canvas_create_offscreen(
  int32_t width,
  int32_t height)
{
  return _canvas_create_internal(CANVAS_OFFSCREEN, false,
                                 false, false, false, false, false,
                                 NULL, 0, 0, width, height, NULL);
}

canvas_t *
canvas_create_offscreen_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  return _canvas_create_internal(CANVAS_OFFSCREEN, false,
                                 false, false, false, false, false,
                                 NULL, 0, 0, pixmap->width, pixmap->height,
                                 pixmap);
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
  return _canvas_create_internal(CANVAS_OFFSCREEN, false,
                                 false, false, false, false, false,
                                 NULL, 0, 0, pixmap.width, pixmap.height,
                                 &pixmap);
}

static void (*_canvas_destroy_callback)(canvas_t *) = NULL;

void
canvas_set_destroy_callback(
  void (*callback_function)(canvas_t *))
{
  _canvas_destroy_callback = callback_function;
}

static void
_canvas_destroy(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->context != NULL);
  assert(canvas->state != NULL);
  assert(canvas->state_stack != NULL);
  assert(canvas->path_2d != NULL);

  if (_canvas_destroy_callback != NULL) {
    _canvas_destroy_callback(canvas);
  }

  backend_remove_canvas(canvas);

  context_destroy(canvas->context);

  /* Offscreen and closed canvas do not have windows */
  if (canvas->window != NULL) {
    window_destroy(canvas->window);
  }

  if (canvas->font != NULL) {
    font_destroy(canvas->font);
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
  assert(canvas->context != NULL);

  if (canvas->window != NULL) {
    window_show(canvas->window);
  }
}

void
canvas_hide(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->context != NULL);

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



/* Rendering */

void
canvas_commit(
  canvas_t *canvas)
{
  assert(canvas != NULL);

  if (canvas->window != NULL) {
    canvas->committed = true;
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

pair_t(int32_t)
canvas_get_size(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->context != NULL);

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

  context_clear_clip(canvas->context);
  canvas->clip_region_dirty = false;
}

void
_canvas_set_size_internal(
  canvas_t *canvas,
  int32_t width,
  int32_t height)
{
  assert(canvas != NULL);
  assert(canvas->context != NULL);

  _canvas_reset_state(canvas);

  width = max(1, width);
  height = max(1, height);

  if (!context_resize(canvas->context, width, height)) {
    return;
  }

  canvas->width = width;
  canvas->height = height;

  if (canvas->window != NULL) {
// should probably perform an internal present event instead
// unless this is already requested internally
// in particular, on macOS, we can't present when we want
    context_present(canvas->context);
  }
}

void
canvas_set_size(
  canvas_t *canvas,
  int32_t width,
  int32_t height)
{
  assert(canvas != NULL);
  assert(canvas->context != NULL);

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
  assert(canvas->context != NULL);

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
  assert(canvas->context != NULL);

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
    context_clear_clip(canvas->context);
    canvas->clip_region_dirty = !list_is_empty(canvas->state->clip_path);
  }
}



/* Transform */

void
canvas_set_transform(
  canvas_t *canvas,
  const transform_t *transform)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(transform != NULL);

  *canvas->state->transform = *transform;
}

void
canvas_transform(
  canvas_t *canvas,
  const transform_t *transform)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);
  assert(transform != NULL);

  transform_mul(canvas->state->transform, transform);
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
canvas_get_miter_limit(
  const canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  return canvas->state->miter_limit;
}

void
canvas_set_miter_limit(
  canvas_t *canvas,
  double miter_limit)
{
  assert(canvas != NULL);
  assert(canvas->state != NULL);

  if (miter_limit <= 0.0) {
    return;
  }

  canvas->state->miter_limit = miter_limit;
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
}

color_t_
canvas_get_shadow_color(
  const canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);

  return c->state->shadow.color;
}

void
canvas_set_shadow_color(
  canvas_t *c,
  color_t_ color)
{
  assert(c != NULL);
  assert(c->state != NULL);

  c->state->shadow.color = color;
}

double
canvas_get_shadow_blur(
  const canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);

  return c->state->shadow.blur;
}

void
canvas_set_shadow_blur(
  canvas_t *c,
  double shadow_blur)
{
  assert(c != NULL);
  assert(c->state != NULL);

  c->state->shadow.blur = shadow_blur;
}

pair_t(double)
canvas_get_shadow_offset(
  const canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);

  return pair(double, c->state->shadow.offset_x, c->state->shadow.offset_y);
}

void
canvas_set_shadow_offset(
  canvas_t *c,
  double x,
  double y)
{
  assert(c != NULL);
  assert(c->state != NULL);

  c->state->shadow.offset_x = x;
  c->state->shadow.offset_y = y;
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

  path2d_quadratic_curve_to(c->path_2d, cpx, cpy, x, y,
                            c->state->transform);
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

  path2d_ellipse(c->path_2d, x, y, rx, ry, r, di, df, ccw,
                 c->state->transform);
}



/* Path stroking/filling */

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

  if (context_clip(c->context, c->state->clip_path,
                   c->state->transform) == false) {
    return false;
  }

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
  assert(c->context != NULL);

  _canvas_clip_region_ensure(c);

// TODO: initial size according to number of primitive
  polygon_t *p = polygon_create(1024, 16);
  if (p == NULL) {
    return;
  }

  rect_t bbox = { 0 };
  if (polygonize(path2d_get_path(c->path_2d), p, &bbox) == true) {
    context_render_polygon(c->context, p, &bbox, c->state->fill_style,
                           c->state->global_alpha, &c->state->shadow,
                           c->state->global_composite_operation,
                           non_zero, c->state->transform);
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
  assert(c->context != NULL);
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

    context_render_polygon(c->context, p, &bbox, c->state->fill_style,
                           c->state->global_alpha, &c->state->shadow,
                           c->state->global_composite_operation,
                           non_zero, c->state->transform);
  }

  polygon_destroy(p);
}

void
canvas_stroke(
  canvas_t *c)
{
  assert(c != NULL);
  assert(c->state != NULL);
  assert(c->context != NULL);
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
                         c->state->miter_limit,
                         c->state->transform, true,
                         c->state->line_dash, c->state->line_dash_len,
                         c->state->line_dash_offset) == true) {
    context_render_polygon(c->context, p, &bbox, c->state->stroke_style,
                           c->state->global_alpha, &c->state->shadow,
                           c->state->global_composite_operation,
                           true, c->state->transform);
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
  assert(c->context != NULL);
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
                         c->state->miter_limit,
                         c->state->transform, false,
                         c->state->line_dash, c->state->line_dash_len,
                         c->state->line_dash_offset) == true) {
    context_render_polygon(c->context, p, &bbox, c->state->stroke_style,
                           c->state->global_alpha, &c->state->shadow,
                           c->state->global_composite_operation,
                           true, c->state->transform);
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

  context_render_polygon(c->context, p, &bbox, c->state->fill_style,
                         c->state->global_alpha, &c->state->shadow,
                         c->state->global_composite_operation,
                         false, c->state->transform);

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
                 c->state->miter_limit,
                 c->state->transform, true, c->state->line_dash,
                 c->state->line_dash_len, c->state->line_dash_offset);

  context_render_polygon(c->context, p, &bbox, c->state->stroke_style,
                         c->state->global_alpha, &c->state->shadow,
                         c->state->global_composite_operation,
                         true, c->state->transform);

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
      context_render_polygon(c->context, p, &bbox, c->state->fill_style,
                             c->state->global_alpha, &c->state->shadow,
                             c->state->global_composite_operation,
                             true, c->state->transform);
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
      context_render_polygon(c->context, p, &bbox, c->state->stroke_style,
                             c->state->global_alpha, &c->state->shadow,
                             c->state->global_composite_operation,
                             true, c->state->transform);
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
  assert(dc->context != NULL);
  assert(dc->state != NULL);
  assert(dc->state->transform != NULL);
  assert(sc != NULL);
  assert(sc->context != NULL);

  context_blit(dc->context, dx, dy,
               sc->context, sx, sy, width, height,
               dc->state->global_alpha, &dc->state->shadow,
               dc->state->global_composite_operation,
               dc->state->transform);
}

/* Direct pixel access */

color_t_
canvas_get_pixel(
  const canvas_t *c,
  int32_t x,
  int32_t y)
{
  assert(c != NULL);
  assert(c->context != NULL);

  return context_get_pixel(c->context, x, y);
}

void
canvas_put_pixel(
  canvas_t *c,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(c != NULL);
  assert(c->context != NULL);

  context_put_pixel(c->context, x, y, color);
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
  assert(c->context != NULL);

  return context_get_pixmap(c->context, sx, sy, width, height);
}

void
canvas_put_pixmap(
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
  assert(c->context != NULL);
  assert(sp != NULL);
  assert(pixmap_valid(*sp) == true);

  context_put_pixmap(c->context, dx, dy, sp, sx, sy, width, height);
}

/* Import / export functions */

bool
canvas_export_png(
  const canvas_t *c,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(c->context != NULL);
  assert(filename != NULL);

  return context_export_png(c->context, filename);
}

bool
canvas_import_png(
  canvas_t *c,
  int32_t x,
  int32_t y,
  const char *filename) // as UTF-8
{
  assert(c != NULL);
  assert(c->context != NULL);
  assert(filename != NULL);

  return context_import_png(c->context, x, y, filename);
}
