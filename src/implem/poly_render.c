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
#include <math.h>
#include <assert.h>

#include "util.h"
#include "point.h"
#include "rect.h"
#include "color.h"
#include "transform.h"
#include "color_composition.h"
#include "image_interpolation.h"
#include "gradient.h"
#include "gradient_internal.h"
#include "pattern.h"
#include "draw_style.h"
#include "polygon.h"
#include "polygon_internal.h"
#include "pixmap.h"
#include "filters.h"
#include "state.h" // just shadow

// Mask array
static uint64_t _masks[(9 * 9) * (9 * 9)] = { 0 };

// Initial counter map
static uint64_t map[256] = { 0 };

// TODO: use symmetries to reduce memory usage / cache misses
void
poly_render_init(
  void)
{
  // Initialize mask array
  uint64_t *mask = _masks;
  for (int x1 = 0; x1 <= 8; ++x1) {
    for (int y1 = 0; y1 <= 8; ++y1) {
      for (int x2 = 0; x2 <= 8; ++x2) {
        for (int y2 = 0; y2 <= 8; ++y2) {
          uint64_t m = 0;
          if (y1 != y2) {
            for (int i = 0; i < 8; ++i) {
              if (((i < y1) && (i < y2)) ||
                  ((i >= y1) && (i >= y2)))
                continue;
              int j = fastround((float)x1 +
                               (float)(x2 - x1) *
                               ((float)i + 0.5f - (float)y1) /
                               (float)(y2 - y1));
              m |= (uint64_t)(0xff >> j) << ((7 - i) * 8);
            }
          }
          *mask++ = m;
        }
      }
    }
  }

  // Initialize initial counter map
  for (int i = 0; i < 256; ++i) {
    map[i] =
      (i & 0x01) * (0x0000000000000001 / 0x01) |
      (i & 0x02) * (0x0000000000000100 / 0x02) |
      (i & 0x04) * (0x0000000000010000 / 0x04) |
      (i & 0x08) * (0x0000000001000000 / 0x08) |
      (i & 0x10) * (0x0000000100000000 / 0x10) |
      (i & 0x20) * (0x0000010000000000 / 0x20) |
      (i & 0x40) * (0x0001000000000000 / 0x40) |
      (i & 0x80) * (0x0100000000000000 / 0x80);
  }
}

static void
_clip_horizontal(
  float y,
  float norm,
  const polygon_t *p,
  polygon_t *np,
  float x_offset,
  float y_offset)
{
  assert(p != NULL);
  assert(np != NULL);
  assert(y >= 0.0f);
  assert((norm == -1.0f) || (norm == 1.0f));

  polygon_reset(np);

  int i = 0;
  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    // i is the current vertex, j is the previous
    for (int j = p->subpolys[ip]; i <= p->subpolys[ip]; j = i, i++) {

      point_t current = p->points[i];
      point_t previous = p->points[j];
      current.x += x_offset;
      current.y += y_offset;
      previous.x += x_offset;
      previous.y += y_offset;

      // If current is on the correct side
      if (((float)current.y - y) * norm <= 0.0f) {
        // If previous is NOT on correct side
        if (((float)previous.y - y) * norm > 0.0f) {
          // Calculate the intercept
          float px1 =
            (float)current.x + (float)(previous.x - current.x) *
            (y - (float)current.y) / (float)(previous.y - current.y);

          // Push the intersect. Don't push the other one,
          // because it's not on the correct side.
          polygon_add_point(np, point(px1, y));
        }

        // Push the current, because it's on the correct side
        polygon_add_point(np, current);
      }
      // If previous is on the correct side
      else if (((float)previous.y - y) * norm <= 0.0f) {
        // Calculate the intercept
        float px1 =
          (float)current.x + (float)(previous.x - current.x) *
          (y - (float)current.y) / (float)(previous.y - current.y);

        // Push the intersect
        polygon_add_point(np, point(px1, y));
      }
    }

    polygon_end_subpoly(np, false);
  }
}

static void
_clip_vertical(
  float x,
  float norm,
  const polygon_t *p,
  polygon_t *np,
  float x_offset,
  float y_offset)
{
  assert(p != NULL);
  assert(np != NULL);
  assert(x >= 0.0f);
  assert((norm == -1.0f) || (norm == 1.0f));

  polygon_reset(np);

  int i = 0;
  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

       // i is the current vertex, j is the previous
    for (int j = p->subpolys[ip]; i <= p->subpolys[ip]; j = i, i++) {

      point_t current = p->points[i];
      point_t previous = p->points[j];
      current.x += x_offset;
      current.y += y_offset;
      previous.x += x_offset;
      previous.y += y_offset;

      // If current is on the correct side
      if (((float)current.x - x) * norm <= 0.0f) {
        // If previous is NOT on correct side
        if (((float)previous.x - x) * norm > 0.0f) {
          // Calculate the intercept
          float py1 =
            (float)current.y + (float)(previous.y - current.y) *
            (x - (float)current.x) / (float)(previous.x - current.x);

          // Push the intersect. Don't push the other one,
          // because it's not on the correct side.
          polygon_add_point(np, point(x, py1));
        }

        // Push the current, because it's on the correct side
        polygon_add_point(np, current);
      }
      // If previous is on the correct side
      else if (((float)previous.x - x) * norm <= 0.0f) {
        // Calculate the intercept
        float py1 =
          (float)current.y + (float)(previous.y - current.y) *
          (x - (float)current.x) / (float)(previous.x - current.x);

        // Push the intersect
        polygon_add_point(np, point(x, py1));
      }
    }

    polygon_end_subpoly(np, false);
  }
}

static int
_calculate_coverage_even_odd(
  float y,
  float x,
  const polygon_t *p)
{
  assert(p != NULL);
  assert(x >= 0.0f);
  assert(y >= 0.0f);

  uint64_t mask = 0;

  int i = 0;
  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    for (int j = p->subpolys[ip]; i <= p->subpolys[ip]; j = i, i++) {

      point_t current = p->points[i];
      point_t previous = p->points[j];

      // TODO: check if useful
//      if (current.y == previous.y) continue;

      int x1 = fastround(((float)current.x - x) * 8.0f);
      int y1 = fastround(((float)current.y - y) * 8.0f);
      int x2 = fastround(((float)previous.x - x) * 8.0f);
      int y2 = fastround(((float)previous.y - y) * 8.0f);

//      if (y1 == y2) continue;

      mask ^= _masks[((x1 * 9 + y1) * 9 + x2) * 9 + y2];
    }

  }

  return (numbits(mask) * 255) / 64;
}


static int
_calculate_coverage_non_zero(
  float y,
  float x,
  const polygon_t *p)
{
  assert(p != NULL);
  assert(x >= 0.0f);
  assert(y >= 0.0f);

  // When too many points, fall back to even_odd
  if (p->nb_points >= 128) {
    return _calculate_coverage_even_odd(y, x, p);
  }

  // 8-bit winding counters packed as 64-bit integers
  uint64_t lcnt[8] = {
    0x8080808080808080, 0x8080808080808080,
    0x8080808080808080, 0x8080808080808080,
    0x8080808080808080, 0x8080808080808080,
    0x8080808080808080, 0x8080808080808080,
  };

  int i = 0;
  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    for (int j = p->subpolys[ip]; i <= p->subpolys[ip]; j = i, i++) {

      point_t current = p->points[i];
      point_t previous = p->points[j];

//      if (current.y == previous.y) continue;

      int x1 = fastround(((float)current.x - x) * 8.0f);
      int y1 = fastround(((float)current.y - y) * 8.0f);
      int x2 = fastround(((float)previous.x - x) * 8.0f);
      int y2 = fastround(((float)previous.y - y) * 8.0f);

//      if (y1 == y2) continue;

      uint64_t m = _masks[((x1 * 9 + y1) * 9 + x2) * 9 + y2];
      if (y2 > y1) {
        lcnt[0] += map[(m & 0x00000000000000FF) >> 0x00];
        lcnt[1] += map[(m & 0x000000000000FF00) >> 0x08];
        lcnt[2] += map[(m & 0x0000000000FF0000) >> 0x10];
        lcnt[3] += map[(m & 0x00000000FF000000) >> 0x18];
        lcnt[4] += map[(m & 0x000000FF00000000) >> 0x20];
        lcnt[5] += map[(m & 0x0000FF0000000000) >> 0x28];
        lcnt[6] += map[(m & 0x00FF000000000000) >> 0x30];
        lcnt[7] += map[(m & 0xFF00000000000000) >> 0x38];
      } else if (y2 < y1) {
        lcnt[0] -= map[(m & 0x00000000000000FF) >> 0x00];
        lcnt[1] -= map[(m & 0x000000000000FF00) >> 0x08];
        lcnt[2] -= map[(m & 0x0000000000FF0000) >> 0x10];
        lcnt[3] -= map[(m & 0x00000000FF000000) >> 0x18];
        lcnt[4] -= map[(m & 0x000000FF00000000) >> 0x20];
        lcnt[5] -= map[(m & 0x0000FF0000000000) >> 0x28];
        lcnt[6] -= map[(m & 0x00FF000000000000) >> 0x30];
        lcnt[7] -= map[(m & 0xFF00000000000000) >> 0x38];
      };
    }

  }

  int bits = 0;

  for (int l = 0; l < 8; ++l) {
    bits += (lcnt[l] & 0x00000000000000FF) != 0x0000000000000080;
    bits += (lcnt[l] & 0x000000000000FF00) != 0x0000000000008000;
    bits += (lcnt[l] & 0x0000000000FF0000) != 0x0000000000800000;
    bits += (lcnt[l] & 0x00000000FF000000) != 0x0000000080000000;
    bits += (lcnt[l] & 0x000000FF00000000) != 0x0000008000000000;
    bits += (lcnt[l] & 0x0000FF0000000000) != 0x0000800000000000;
    bits += (lcnt[l] & 0x00FF000000000000) != 0x0080000000000000;
    bits += (lcnt[l] & 0xFF00000000000000) != 0x8000000000000000;
  }

  return bits * 255 / 64;
}


static bool *
_build_complex(
  int32_t w,
  const polygon_t *p)
{
  assert(p != NULL);
  bool *complex = (bool *)calloc(w, sizeof(bool));

  int i = 0;
  for (int ip = 0; ip < p->nb_subpolys; ++ip) {

    for (int j = p->subpolys[ip]; i <= p->subpolys[ip]; j = i, i++) {

      if (p->points[i].y != p->points[j].y) {
        int ix1 = (int)floor(p->points[i].x);
        int ix2 = (int)floor(p->points[j].x);
        if (ix1 > ix2) {
          swap(int, ix1, ix2);
        }

        if (ix1 < w && ix2 >= 0) { // Are any pixels affected?

          if (ix2 >= w) { // Clamp to the complex table
            ix2 = w - 1;
          }
          if (ix1 < 0) { // Clamp to the complex table
            ix1 = 0;
          }

          for (int x = ix1; x <= ix2; ++x) {
            complex[x] = true;
          }
        }
      }
    }
  }

  return complex;
}

static color_t_
_determine_base_color(
  const draw_style_t *draw_style,
  float x,
  float y,
  const transform_t *inv)
{
  assert(draw_style != NULL);
  assert(inv != NULL);

  color_t_ color = color_transparent_black;

  switch (draw_style->type) {
    case DRAW_STYLE_COLOR:
      color = draw_style->content.color;
      break;
    case DRAW_STYLE_GRADIENT:
      color = gradient_evaluate_pos(draw_style->content.gradient, x, y, inv);
      break;
    case DRAW_STYLE_PATTERN:
      color = pattern_evaluate_pos(draw_style->content.pattern, x, y, inv);
      break;
    case DRAW_STYLE_PIXMAP: {
        point_t p = point(x, y);
        transform_apply(inv, &p);
        p.x = max(0, min(draw_style->content.pixmap->width - 1, p.x));
        p.y = max(0, min(draw_style->content.pixmap->height - 1, p.y));
        color = interpolation_cubic(draw_style->content.pixmap, p.x, p.y);
        break;
      }
    default:
      assert(!"Invalid draw style");
      break;
  }

  return color;
}

static pixmap_t
_poly_render_pixmap(
  const polygon_t *p,
  const rect_t *bbox,
  const draw_style_t draw_style,
  const transform_t *transform,
  bool non_zero)
{
  assert(p != NULL);
  assert(bbox != NULL);
  assert(transform != NULL);
  assert((draw_style.type != DRAW_STYLE_GRADIENT) ||
         (draw_style.content.gradient != NULL));
  assert((draw_style.type != DRAW_STYLE_PATTERN) ||
         (draw_style.content.pattern != NULL));
  assert(transform != NULL);

  int alpha = 0;

  polygon_t *line_poly = polygon_create(1024, 16);
  polygon_t *pixel_poly = polygon_create(1024, 16);
  polygon_t *tmp_poly = polygon_create(1024, 16);

  transform_t *inverse = transform_copy(transform);
  transform_inverse(inverse);

  int32_t w = (int32_t)(bbox->p2.x - bbox->p1.x) + 1;
  int32_t h = (int32_t)(bbox->p2.y - bbox->p1.y) + 1;

  pixmap_t pm = pixmap(w, h, NULL);

  for (int32_t i = 0; i < h; i++) {

    _clip_horizontal((float)i, -1.0, p, tmp_poly, -bbox->p1.x, -bbox->p1.y);
    _clip_horizontal((float)(i + 1), 1.0, tmp_poly, line_poly, 0.0, 0.0);

    bool *complex = _build_complex(w, line_poly);
    bool calculate = true;

    // Calculate scanline
    for (int32_t j = 0; j < w; j++) {

      bool is_complex = complex[j];

      // If the current cell is complex, we need to calculate it
      calculate |= is_complex;

      if (calculate) {
        _clip_vertical((float)j, -1.0, line_poly, tmp_poly, 0.0, 0.0);
        _clip_vertical((float)(j + 1), 1.0, tmp_poly, pixel_poly, 0.0, 0.0);

        swap(polygon_t *, line_poly, tmp_poly);

        alpha =
          non_zero ?
          _calculate_coverage_non_zero((float)i, (float)j, pixel_poly) :
          _calculate_coverage_even_odd((float)i, (float)j, pixel_poly);

        // Our coverage can be used in the next pixel
        // only if this pixel is simple
        calculate = is_complex;
      }

      // Determine the pixel base color according to draw style
      color_t_ color = _determine_base_color(&draw_style, (float)j + bbox->p1.x,
                                             (float)i + bbox->p1.y, inverse);

      int draw_alpha = (alpha * color.a) / 255;
      pixmap_at(pm, i, j) = color(draw_alpha, color.r, color.g, color.b);
    }

    free(complex);
  }

  transform_destroy(inverse);

  polygon_destroy(tmp_poly);
  polygon_destroy(pixel_poly);
  polygon_destroy(line_poly);

  return pm;
}

static void
_poly_render_layered(
  pixmap_t *pm,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  composite_operation_t composite_operation,
  const shadow_t *shadow,
  double global_alpha,
  const pixmap_t *clip_region,
  bool non_zero,
  const transform_t *transform)
{
  assert(pm != NULL);
  assert(pixmap_valid(*pm) == true);
  assert(p != NULL);
  assert(bbox != NULL);
  assert((draw_style.type != DRAW_STYLE_GRADIENT) ||
         (draw_style.content.gradient != NULL));
  assert((draw_style.type != DRAW_STYLE_PATTERN) ||
         (draw_style.content.pattern != NULL));
  assert(shadow != NULL);
  assert(transform != NULL);

  pixmap_t rendered_poly =
    _poly_render_pixmap(p, bbox, draw_style, transform, non_zero);

  // Compose shadows if any
  if ((shadow->blur > 0.0 ||
       shadow->offset_x != 0.0 || shadow->offset_y != 0.0) &&
      composite_operation != COPY && shadow->color.a != 0) {

    int shadow_size_offset =
      (int)(sqrt(3.0 * shadow->blur * shadow->blur));

    pixmap_t shadow_poly =
      pixmap(rendered_poly.width + shadow_size_offset * 2,
             rendered_poly.height + shadow_size_offset * 2, NULL);
    for (int32_t i = 0; i < rendered_poly.width; i++) {
      for (int32_t j = 0; j < rendered_poly.height; j++) {
        pixmap_at(shadow_poly,
                  j + shadow_size_offset,
                  i + shadow_size_offset).a =
          pixmap_at(rendered_poly, j, i).a;
      }
    }

    pixmap_t blurred_shadow_poly;
    if (shadow->blur == 0.0) {
      blurred_shadow_poly = shadow_poly;
    } else {
      blurred_shadow_poly =
        filter_gaussian_blur_alpha(&shadow_poly, shadow->blur / 2.0);
      pixmap_destroy(shadow_poly);
    }

    rect_t sbbox =
      rect(point(bbox->p1.x - shadow_size_offset + shadow->offset_x,
                 bbox->p1.y - shadow_size_offset + shadow->offset_y),
           point(bbox->p2.x + shadow_size_offset + shadow->offset_x,
                 bbox->p2.y + shadow_size_offset + shadow->offset_y));

    int32_t lower_bound_i = 0, upper_bound_i = pm->height;
    int32_t lower_bound_j = 0, upper_bound_j = pm->width;

    if (comp_is_full_screen(composite_operation) == false) {
      lower_bound_i = max((int32_t)sbbox.p1.y, 0);
      upper_bound_i = min((int32_t)(sbbox.p2.y + 1.0), pm->height);
      lower_bound_j = max((int32_t)sbbox.p1.x, 0);
      upper_bound_j = min((int32_t)(sbbox.p2.x + 1.0), pm->width);
    }

    for (int32_t i = lower_bound_i; i < upper_bound_i; ++i) {
      for (int32_t j = lower_bound_j; j < upper_bound_j; ++j) {

        if (j < sbbox.p1.x || j > sbbox.p2.x ||
            i < sbbox.p1.y || i > sbbox.p2.y) {
          pixmap_at(*pm, i, j) =
            comp_compose(color_transparent_black,
                         pixmap_at(*pm, i, j), 0, composite_operation);
          continue;
        }

        color_t_ fill_color =
          pixmap_at(blurred_shadow_poly,
                    i - (int32_t)sbbox.p1.y,
                    j - (int32_t)sbbox.p1.x);

        fill_color.r = shadow->color.r;
        fill_color.g = shadow->color.g;
        fill_color.b = shadow->color.b;

        double draw_alpha = fill_color.a;

        if ((clip_region != NULL) && (pixmap_valid(*clip_region) == true)) {
          draw_alpha *= 255 - pixmap_at(*clip_region, i, j).a;
          draw_alpha /= 255;
        }

        pixmap_at(*pm, i, j) =
          comp_compose(fill_color, pixmap_at(*pm, i, j),
                       (int)(draw_alpha * shadow->color.a *
                             global_alpha / 255), composite_operation);
      }
    }

    pixmap_destroy(blurred_shadow_poly);
  }

  // Compose rendered mesh
  int32_t lower_bound_i = 0, upper_bound_i = pm->height;
  int32_t lower_bound_j = 0, upper_bound_j = pm->width;

  if (comp_is_full_screen(composite_operation) == false) {
    lower_bound_i = max((int32_t)bbox->p1.y, 0);
    upper_bound_i = min((int32_t)(bbox->p2.y + 1.0), pm->height);
    lower_bound_j = max((int32_t)bbox->p1.x, 0);
    upper_bound_j = min((int32_t)(bbox->p2.x + 1.0), pm->width);
  }

  for (int32_t i = lower_bound_i; i < upper_bound_i; ++i) {
    for (int32_t j = lower_bound_j; j < upper_bound_j; ++j) {

      if (j < bbox->p1.x || j > bbox->p2.x ||
          i < bbox->p1.y || i > bbox->p2.y) {
        pixmap_at(*pm, i, j) =
          comp_compose(color_transparent_black,
                       pixmap_at(*pm, i, j), 0, composite_operation);
        continue;
      }

      color_t_ fill_color =
        pixmap_at(rendered_poly,
                  i - (int32_t)bbox->p1.y,
                  j - (int32_t)bbox->p1.x);

      double draw_alpha = fill_color.a;

      if ((clip_region != NULL) && (pixmap_valid(*clip_region) == true)) {
        draw_alpha *= 255 - pixmap_at(*clip_region, i, j).a;
        draw_alpha /= 255;
      }

      pixmap_at(*pm, i, j) =
        comp_compose(fill_color, pixmap_at(*pm, i, j),
                     (int)(draw_alpha * global_alpha),
                     composite_operation);
    }
  }

  pixmap_destroy(rendered_poly);
}

static void
_poly_render_direct(
  pixmap_t *pm,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  composite_operation_t composite_operation,
  double global_alpha,
  const pixmap_t *clip_region,
  bool non_zero,
  const transform_t *transform)
{
  assert(pm != NULL);
  assert(pixmap_valid(*pm) == true);
  assert(p != NULL);
  assert(bbox != NULL);
  assert((draw_style.type != DRAW_STYLE_GRADIENT) ||
         (draw_style.content.gradient != NULL));
  assert((draw_style.type != DRAW_STYLE_PATTERN) ||
         (draw_style.content.pattern != NULL));
  assert(transform != NULL);

  int alpha = 0;

  polygon_t *line_poly = polygon_create(1024, 16);
  polygon_t *pixel_poly = polygon_create(1024, 16);
  polygon_t *tmp_poly = polygon_create(1024, 16);

  transform_t *inverse = transform_copy(transform);
  transform_inverse(inverse);

  int32_t lower_bound_i = 0, upper_bound_i = pm->height;
  int32_t lower_bound_j = 0, upper_bound_j = pm->width;

  if (comp_is_full_screen(composite_operation) == false) {
    lower_bound_i = max((int32_t)bbox->p1.y, 0);
    upper_bound_i = min((int32_t)(bbox->p2.y + 1.0), pm->height);
    lower_bound_j = max((int32_t)bbox->p1.x, 0);
    upper_bound_j = min((int32_t)(bbox->p2.x + 1.0), pm->width);
  }

  for (int32_t i = lower_bound_i; i < upper_bound_i; ++i) {

    // If not in the bounding box, take src color as transparent black
    if (i < bbox->p1.y || i > bbox->p2.y) {
      for (int32_t j = 0; j < pm->width; ++j) {
        pixmap_at(*pm, i, j) = comp_compose(color_transparent_black,
                                            pixmap_at(*pm, i, j), 0,
                                            composite_operation);
      }
      continue;
    }

    _clip_horizontal((float)i, -1.0, p, tmp_poly, 0.0, 0.0);
    _clip_horizontal((float)(i + 1), 1.0, tmp_poly, line_poly, 0.0, 0.0);

    bool *complex = _build_complex(pm->width , line_poly);
    bool calculate = true;

    // Calculate scanline, bounded by the bounding box
    for (int32_t j = lower_bound_j; j < upper_bound_j; ++j) {

      // If not in the bounding box, take src color as transparent black
      if (j < bbox->p1.x || j > bbox->p2.x) {
        pixmap_at(*pm, i, j) = comp_compose(color_transparent_black,
                                            pixmap_at(*pm, i, j), 0,
                                            composite_operation);
        continue;
      }

      bool is_complex = complex[j];

      // If the current cell is complex, we need to calculate it.
      calculate |= is_complex;

      if (calculate) {
        _clip_vertical((float)j, -1.0, line_poly, tmp_poly, 0.0, 0.0);
        _clip_vertical((float)(j + 1), 1.0, tmp_poly, pixel_poly, 0.0, 0.0);

        swap(polygon_t *, line_poly, tmp_poly);

        alpha =
          non_zero ?
          _calculate_coverage_non_zero((float)i, (float)j, pixel_poly) :
          _calculate_coverage_even_odd((float)i, (float)j, pixel_poly);

        // Our coverage can be used in the next pixel
        // only if this pixel is simple
        calculate = is_complex;
      }

      // Determine the pixel base color according to draw style
      color_t_ color =
        _determine_base_color(&draw_style, (float)j, (float)i, inverse);

      int draw_alpha =
        (alpha * fastround(global_alpha * 256.0) *
         color.a) / (256 * 255);
      if ((clip_region != NULL) && (pixmap_valid(*clip_region) == true)) {
        draw_alpha *= 255 - pixmap_at(*clip_region, i, j).a;
        draw_alpha /= 255;
      }

      // Apply the coverage to the pixel
      // Only do this logic if there's something to apply
      pixmap_at(*pm, i, j) =
        comp_compose(color, pixmap_at(*pm, i, j),
                     draw_alpha, composite_operation);
    }

    free(complex);
  }

  transform_destroy(inverse);

  polygon_destroy(tmp_poly);
  polygon_destroy(pixel_poly);
  polygon_destroy(line_poly);
}


void
poly_render(
  pixmap_t *s,
  const polygon_t *p,
  const rect_t *bbox,
  draw_style_t draw_style,
  double global_alpha,
  const shadow_t *shadow,
  composite_operation_t compose_op,
  const pixmap_t *clip_region,
  bool non_zero,
  const transform_t *transform)
{
  if ((shadow->blur > 0.0 ||
       shadow->offset_x != 0.0 || shadow->offset_y != 0.0) &&
      compose_op != COPY && shadow->color.a != 0) {
    _poly_render_layered(s, p, bbox, draw_style, compose_op, shadow,
                         global_alpha, clip_region, non_zero, transform);
  }
  else {
    _poly_render_direct(s, p, bbox, draw_style, compose_op,
                        global_alpha, clip_region, non_zero, transform);
  }
}
