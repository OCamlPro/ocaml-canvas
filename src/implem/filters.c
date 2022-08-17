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
#include "pixmap.h"
#include "filters.h"

static void
_filter_blur_compute_boxes(
  double s,
  int32_t n,
  int32_t *boxes) // out
{
  assert(boxes != NULL);

  double m_ideal = sqrt(((12.0 * s * s) / (double)n) + 1.0);
  int32_t wl = (int32_t)floor(m_ideal);
  if (wl % 2 == 0) {
    wl--;
  }

  int32_t wu = wl + 2;
  m_ideal =
    (12.0 * s * s - (double)(n * wl * wl - 4 * n * wl - 3 * n)) /
    ((double)(-4 * wl - 4));
  int32_t m = (int32_t)floor(m_ideal);

  for (int32_t i = 0; i < n; ++i) {
    boxes[i] = (i < m) ? wl : wu;
  }
}

static void
_filter_blur_box_h(
  pixmap_t *dst,
  const pixmap_t *src,
  int32_t r)
{
  assert(pixmap_valid(*src));
  assert(pixmap_valid(*dst));
  assert(src->width == dst->width);
  assert(src->height == dst->height);
  assert(r >= 0);

  int32_t w = src->width;
  int32_t h = src->height;
  double iarr = 1.0 / ((double)(2 * r + 1));

  for (int32_t i = 0; i < h; ++i) {
    int32_t ti = i * w, li = ti, ri = ti + r;
    int32_t fv = src->data[ti].a;
    int32_t lv = src->data[ti + w - 1].a;
    int32_t val = (r + 1) * fv;
    for (int32_t j = 0; j < r; ++j) {
      val += src->data[ti + j].a;
    }
    for (int32_t j = 0; j <= r; ++j) {
      val += src->data[ri++].a - fv;
      dst->data[ti++].a = fastround(val * iarr);
    }
    for (int32_t j = r + 1; j < w - r; ++j) {
      val += src->data[ri++].a - src->data[li++].a;
      dst->data[ti++].a = fastround(val * iarr);
    }
    for (int32_t j = w - r; j < w; ++j) {
      val += lv - src->data[li++].a;
      dst->data[ti++].a = fastround(val * iarr);
    }
  }
}

static void
_filter_blur_box_v(
  pixmap_t *dst,
  const pixmap_t *src,
  int32_t r)
{
  assert(pixmap_valid(*src));
  assert(pixmap_valid(*dst));
  assert(src->width == dst->width);
  assert(src->height == dst->height);
  assert(r >= 0);

  int32_t w = src->width;
  int32_t h = src->height;
  double iarr = 1.0 / ((double)(2 * r + 1));

  for (int32_t i = 0; i < w; ++i) {
    int32_t ti = i, li = ti, ri = ti + r * w;
    int32_t fv = src->data[ti].a;
    int32_t lv = src->data[ti + w * (h-1)].a;
    int32_t val = (r + 1) * fv;
    for (int32_t j = 0; j < r; ++j) {
      val += src->data[ti + j*w].a;
    }
    for (int32_t j = 0; j <= r; ++j) {
      val += src->data[ri].a - fv;
      dst->data[ti].a = fastround(val * iarr);
      ri += w;
      ti += w;
    }
    for (int32_t j = r + 1; j < h - r; ++j) {
      val += src->data[ri].a - src->data[li].a;
      dst->data[ti].a = fastround(val * iarr);
      li += w;
      ri += w;
      ti += w;
    }
    for (int32_t j = h - r; j < h; ++j) {
      val += lv - src->data[li].a;
      dst->data[ti].a = fastround(val * iarr);
      li += w;
      ti += w;
    }
  }
}

// Note: dst must be an exact copy of src
static void
_filter_blur_box(
  pixmap_t *src,
  pixmap_t *dst,
  int32_t r)
{
  assert(pixmap_valid(*src));
  assert(pixmap_valid(*dst));

  _filter_blur_box_h(src, dst, r);
  _filter_blur_box_v(dst, src, r);
}

// https://blog.ivank.net/fastest-gaussian-blur.html
pixmap_t
filter_gaussian_blur_alpha(
  pixmap_t *src,
  double s)
{
  pixmap_t dst = pixmap_copy(*src);
  if (pixmap_valid(dst) == true) {
    int32_t boxes[3] = { 0 };
    _filter_blur_compute_boxes(s, 3, boxes);
    _filter_blur_box(src, &dst, (boxes[0] - 1) / 2);
    _filter_blur_box(&dst, src, (boxes[1] - 1) / 2);
    _filter_blur_box(src, &dst, (boxes[2] - 1) / 2);
  }
  return dst;
}
