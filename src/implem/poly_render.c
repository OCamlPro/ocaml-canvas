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
#include "polygon.h"
#include "polygon_internal.h"
#include "surface.h"
#include "surface_internal.h"

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
  polygon_t *np)
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
  polygon_t *np)
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
  const surface_t *s,
  const polygon_t *p)
{
  assert(s != NULL);
  assert(p != NULL);

  int w = s->width;
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

          if (ix2 >= w)	// Clamp to the complex table
            ix2 = w - 1;
          if (ix1 < 0)	// Clamp to the complex table
            ix1 = 0;

          for (int x = ix1; x <= ix2; ++x)
            complex[x] = true;
        }
      }
    }
  }

  return complex;
}

static void
_setpixel(
  surface_t *s,
  int i,
  int j,
  color_t_ color)
{
  assert(s != NULL);
  assert(s->data != NULL);
  assert((i >= 0) && (i < s->height));
  assert((j >= 0) && (j < s->width));

  s->data[i * s->width + j] = color;
}

static void
_setpixel_alpha(
  surface_t *s,
  int i,
  int j,
  int alpha,
  color_t_ fg_color)
{
  assert(s != NULL);
  assert(s->data != NULL);
  assert((i >= 0) && (i < s->height));
  assert((j >= 0) && (j < s->width));

/*
  uint32_t bg_color = s->data[i * s->width + j];
  int bg_r, bg_g, bg_b;
  int fg_r, fg_g, fg_b;
  unpack_rgb(bg_r, bg_g, bg_b, bg_color);
  unpack_rgb(fg_r, fg_g, fg_b, fg_color);
  int r = alpha_blend(alpha, bg_r, fg_r);
  int g = alpha_blend(alpha, bg_g, fg_g);
  int b = alpha_blend(alpha, bg_b, fg_b);
  s->data[i * s->width + j] = 0xFF000000 | pack_rgb(r,g,b);
*/

  color_t_ bg_color = s->data[i * s->width + j];
  s->data[i * s->width + j] = alpha_blend(alpha, bg_color, fg_color);
}

void
poly_render(
  surface_t *s,
  const polygon_t *p,
  const rect_t *bbox,
  color_t_ color,
  double global_alpha,
  bool non_zero)
{
  assert(s != NULL);
  assert(s->data != NULL);
  assert(p != NULL);
  assert(bbox != NULL);

  int alpha = 0;

  polygon_t *line_poly = polygon_create(1024, 16);
  polygon_t *pixel_poly = polygon_create(1024, 16);
  polygon_t *tmp_poly = polygon_create(1024, 16);

  for (int i = max((int)bbox->p1.y, 0);
       i < min((int)(bbox->p2.y + 1.0), s->height); ++i) {

    _clip_horizontal((float)i, -1.0, p, tmp_poly);
    _clip_horizontal((float)(i + 1), 1.0, tmp_poly, line_poly);

    bool *complex = _build_complex(s, line_poly);
    bool calculate = true;

    // Calculate scanline, bounded by the bounding box
    for (int j = max((int)bbox->p1.x, 0);
         j < min((int)(bbox->p2.x + 1.0), s->width); ++j) {

      bool is_complex = complex[j];

      // If the current cell is complex, we need to calculate it.
      calculate |= is_complex;

      if (calculate) {
        _clip_vertical((float)j, -1.0, line_poly, tmp_poly);
        _clip_vertical((float)(j + 1), 1.0, tmp_poly, pixel_poly);

        swap(polygon_t *, line_poly, tmp_poly);

        alpha =
          non_zero ?
          _calculate_coverage_non_zero((float)i, (float)j, pixel_poly) :
          _calculate_coverage_even_odd((float)i, (float)j, pixel_poly);

        // Our coverage can be used in the next pixel
        // only if this pixel is simple
        calculate = is_complex;
      }
      int ga = fastround(global_alpha * 256.0);
      int draw_alpha = alpha * ga / 256;
      // Apply the coverage to the pixel
      // Only do this logic if there's something to apply
      if (draw_alpha == 255) {
        _setpixel(s, i, j, color);
      } else if (draw_alpha != 0) {
        _setpixel_alpha(s, i, j, draw_alpha, color);
      }
    }
    free(complex);
  }

  polygon_destroy(tmp_poly);
  polygon_destroy(pixel_poly);
  polygon_destroy(line_poly);
}
