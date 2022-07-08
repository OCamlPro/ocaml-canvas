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
#include <stdbool.h>
#include <assert.h>

#include "config.h"
#include "point.h"
#include "rect.h"
#include "polygonize.h"
#include "font_desc.h"
#include "font.h"
#include "font_internal.h"

#ifdef HAS_GDI
#include "gdi/gdi_font.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_font.h"
#endif
#if defined HAS_X11 || defined HAS_WAYLAND
#include "freetype/ft_font.h"
#endif

font_t *
font_create(
  font_desc_t *fd)
{
  font_t *f = NULL;

  switch_IMPL() {
    case_GDI(f = (font_t *)gdi_font_create(fd));
    case_QUARTZ(f = (font_t *)qtz_font_create(fd));
    case_X11(f = (font_t *)ft_font_create(fd));
    case_WAYLAND(f = (font_t *)ft_font_create(fd));
    default_fail();
  }

  if (f == NULL) {
    return NULL;
  }

  f->font_desc = font_desc_copy(fd);
  if (f->font_desc == NULL) {
    font_destroy(f);
    return NULL;
  }

  return f;
}

void
font_destroy(
  font_t *f)
{
  assert(f != NULL);

  if (f->font_desc != NULL) {
    font_desc_destroy(f->font_desc);
    f->font_desc = NULL;
  }

  switch_IMPL() {
    case_GDI(gdi_font_destroy((gdi_font_t *)f));
    case_QUARTZ(qtz_font_destroy((qtz_font_t *)f));
    case_X11(ft_font_destroy((ft_font_t *)f));
    case_WAYLAND(ft_font_destroy((ft_font_t *)f));
    default_fail();
  }
}

bool
font_matches(
  const font_t *f,
  const font_desc_t *fd)
{
  assert(f != NULL);
  assert(fd != NULL);

  return font_desc_equal(f->font_desc, fd);
}

bool
font_char_as_poly(
  const font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(f != NULL);
  assert(t != NULL);
  assert(pen != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(c <= 0x10FFFF);

  bool res = false;

  switch_IMPL() {
    case_GDI(
      res = gdi_font_char_as_poly((gdi_font_t *)f, t, c, pen, p, bbox));
    case_QUARTZ(
      res = qtz_font_char_as_poly((qtz_font_t *)f, t, c, pen, p, bbox));
    case_X11(
      res = ft_font_char_as_poly((ft_font_t *)f, t, c, pen, p, bbox));
    case_WAYLAND(
      res = ft_font_char_as_poly((ft_font_t *)f, t, c, pen, p, bbox));
    default_fail();
  }

  return res;
}

bool
font_char_as_poly_outline(
  const font_t *f,
  const transform_t *t,
  uint32_t c,
  double w,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(f != NULL);
  assert(t != NULL);
  assert(w > 0.0);
  assert(pen != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(c <= 0x10FFFF);

  polygon_t *tp = polygon_create(256, 8);
  if (tp == NULL) {
    return false;
  }

  bool res = font_char_as_poly(f, t, c, pen, tp, bbox);
  if (res == false) {
    polygon_destroy(tp);
    return false;
  }

  polygon_offset(tp, p, w, JOIN_ROUND, CAP_BUTT, t, true);

  polygon_destroy(tp);

  bbox->p1.x -= w / 2.0; bbox->p1.y -= w / 2.0;
  bbox->p2.x += w / 2.0; bbox->p2.y += w / 2.0;

  return true;
}
