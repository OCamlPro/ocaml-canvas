/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __CANVAS_H
#define __CANVAS_H

#include <stdint.h>
#include <stdbool.h>

#include "object.h"
#include "tuples.h"
#include "color.h"
#include "pixmap.h"
#include "font_desc.h"
#include "gradient.h"
#include "path2d.h"
#include "draw_style.h"
#include "polygonize.h"
#include "color_composition.h"

typedef struct canvas_t canvas_t;

typedef enum canvas_type_t {
  CANVAS_OFFSCREEN = 0,
  CANVAS_FRAMED    = 1,
  CANVAS_FRAMELESS = 2
} canvas_type_t;

DECLARE_OBJECT_METHODS(canvas_t, canvas)

canvas_t *
canvas_create_framed(
  const char *title,
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

canvas_t *
canvas_create_frameless(
  int32_t x,
  int32_t y,
  int32_t width,
  int32_t height);

canvas_t *
canvas_create_offscreen(
  int32_t width,
  int32_t height);

// Creates a canvas from a pixmap
// The data pointer is transfered to the canvas; do NOT free it
canvas_t *
canvas_create_offscreen_from_pixmap(
  pixmap_t *pixmap);

canvas_t *
canvas_create_offscreen_from_png(
  const char *filename);

void
canvas_set_destroy_callback(
  void (*callback)(canvas_t *));



/* Visibility */

void
canvas_show(
  canvas_t *canvas);

void
canvas_hide(
  canvas_t *canvas);

void canvas_close(
  canvas_t *canvas);

bool canvas_is_closed(
  const canvas_t *canvas);



/* Configuration */

canvas_type_t
canvas_get_type(
  const canvas_t *canvas);

int32_t
canvas_get_id(
  const canvas_t *canvas);

pair_t(int32_t)
canvas_get_size(
  const canvas_t *canvas);

void
canvas_set_size(
  canvas_t *canvas,
  int32_t width,
  int32_t height);

void
_canvas_set_size_internal(
  canvas_t *canvas,
  int32_t width,
  int32_t height);

pair_t(int32_t)
canvas_get_position(
  const canvas_t *canvas);

void
canvas_set_position(
  canvas_t *canvas,
  int32_t x,
  int32_t y);



/* State */

bool
canvas_save(
  canvas_t *canvas);

void
canvas_restore(
  canvas_t *canvas);



/* Transform */

void
canvas_set_transform(
  canvas_t *canvas,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f);

void
canvas_transform(
  canvas_t *canvas,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f);

void
canvas_translate(
  canvas_t *canvas,
  double x,
  double y);

void
canvas_scale(
  canvas_t *canvas,
  double x,
  double y);

void
canvas_shear(
  canvas_t *canvas,
  double x,
  double y);

void
canvas_rotate(
  canvas_t *canvas,
  double a);



/* Style / config */

uint32_t
canvas_get_line_width(
  const canvas_t *canvas);

void
canvas_set_line_width(
  canvas_t *canvas,
  double line_width);

join_type_t
canvas_get_join_type(
  const canvas_t *canvas);

void
canvas_set_join_type(
  canvas_t *canvas,
  join_type_t join_type);

cap_type_t
canvas_get_cap_type(
  const canvas_t *canvas);

void
canvas_set_cap_type(
  canvas_t *canvas,
  cap_type_t cap_type);

double
canvas_get_line_dash_offset(
  const canvas_t *canvas);

void
canvas_set_line_dash_offset(
  canvas_t *canvas,
  double line_dash_offset);

// Direct access to the line dash array
// Do NOT free the returned pointer !
const double *
canvas_get_line_dash(
  const canvas_t *canvas);

size_t
canvas_get_line_dash_length(
  const canvas_t *canvas);

// Stores a line dash array (NULL is accepted)
// The provided array is copied
void
canvas_set_line_dash(
  canvas_t *canvas,
  const double *dash,
  size_t n);

color_t_
canvas_get_stroke_color(
  const canvas_t *c);

void
canvas_set_stroke_color(
  canvas_t *c,
  color_t_ color);

draw_style_t
canvas_get_stroke_style(
  const canvas_t *c);

// Stores the gradient to use for stroking lines
// The provided gradient reference count is increased
void
canvas_set_stroke_gradient(
  canvas_t *c,
  gradient_t *gradient);

void
canvas_set_stroke_pattern(
  canvas_t *c,
  pattern_t *pattern);

color_t_
canvas_get_fill_color(
  const canvas_t *c);

void
canvas_set_fill_color(
  canvas_t *c,
  color_t_ color);

draw_style_t
canvas_get_fill_style(
  const canvas_t *c);

// Stores the gradient to use for filling polygons
// The provided gradient reference count is increased
void
canvas_set_fill_gradient(
  canvas_t *c,
  gradient_t *gradient);

void
canvas_set_fill_pattern(
  canvas_t *c,
  pattern_t *pattern);

double
canvas_get_global_alpha(
  const canvas_t *c);

void
canvas_set_global_alpha(
  canvas_t *c,
  double global_alpha);

composite_operation_t
canvas_get_comp_operation(
  canvas_t *c);

void
canvas_set_comp_operation(
  canvas_t *c,
  composite_operation_t op);

// Sets the canvas font
// The provided font name is copied
void
canvas_set_font(
  canvas_t *c,
  const char *family,
  double size,
  font_slant_t slant,
  int32_t weight);



/* Paths */

bool
canvas_clear_path(
  canvas_t *c);

void
canvas_close_path(
  canvas_t *c);

void
canvas_move_to(
  canvas_t *c,
  double x,
  double y);

void
canvas_line_to(
  canvas_t *c,
  double x,
  double y);

void
canvas_arc(
  canvas_t *c,
  double x,
  double y,
  double r,
  double di,
  double df,
  bool ccw);

void
canvas_arc_to(
  canvas_t *c,
  double x1,
  double y1,
  double x2,
  double y2,
  double r);

void
canvas_quadratic_curve_to(
  canvas_t *c,
  double cp1x,
  double cp1y,
  double x,
  double y);

void
canvas_bezier_curve_to(
  canvas_t *c,
  double cp1x,
  double cp1y,
  double cp2x,
  double cp2y,
  double x,
  double y);

void
canvas_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height);

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
  bool ccw);



/* Path stroking/filling */

void
canvas_fill(
  canvas_t *c,
  bool non_zero);

void
canvas_fill_path(
  canvas_t *c,
  path2d_t *path,
  bool non_zero);

void
canvas_stroke(
  canvas_t *c);

void
canvas_stroke_path(
  canvas_t *c,
  path2d_t *path);

void
canvas_clip(
  canvas_t *c,
  bool non_zero);

void 
canvas_clip_path(
  canvas_t *c,
  path2d_t *path,
  bool non_zero);

/* Immediate drawing */

void
canvas_fill_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height);

void
canvas_stroke_rect(
  canvas_t *c,
  double x,
  double y,
  double width,
  double height);

void
canvas_fill_text(
  canvas_t *c,
  const char *text,
  double x,
  double y,
  double max_width);

void
canvas_stroke_text(
  canvas_t *c,
  const char *text,
  double x,
  double y,
  double max_width);

void
canvas_blit(
  canvas_t *dc,
  int32_t dx,
  int32_t dy,
  const canvas_t *sc,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);



/* Direct pixel access */

color_t_
canvas_get_pixel(
  const canvas_t *c,
  int32_t x,
  int32_t y);

void
canvas_set_pixel(
  canvas_t *c,
  int32_t x,
  int32_t y,
  color_t_ color);

// Creates a copy of the surface pixels
// Be sure to free the data pointer when done
pixmap_t
canvas_get_pixmap(
  const canvas_t *c,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

void
canvas_set_pixmap(
  canvas_t *c,
  int32_t dx,
  int32_t dy,
  const pixmap_t *data,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);



/* Import / export functions */

bool
canvas_export_png(
  const canvas_t *c,
  const char *filename);

bool
canvas_import_png(
  canvas_t *c,
  int32_t x,
  int32_t y,
  const char *filename);

#endif /* __CANVAS_H */
