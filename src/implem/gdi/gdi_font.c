/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_GDI

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <windows.h>

#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../polygonize.h"
#include "../transform.h"
#include "../font_desc_internal.h"
#include "gdi_font_internal.h"

static bool
_gdi_font_slant(
  font_slant_t s)
{
  switch (s) {
    case SLANT_ROMAN: return false;
    case SLANT_ITALIC: return true;
    case SLANT_OBLIQUE: return true; /* Approximate */
    default: assert(!"Invalid font slant"); return 0;
  }
}

gdi_font_t *
gdi_font_create(
  font_desc_t *fd)
{
  assert(fd != NULL);
  assert(fd->weight >= 0);

  const char *family = fd->family;
  if (family == NULL) {
    family = "Arial";
  }

  double size = fd->size;
  if (size <= 0.0) {
    size = 8.0;
  }

  gdi_font_t *f = (gdi_font_t *)calloc(1, sizeof(gdi_font_t));
  if (f == NULL) {
    return NULL;
  }

  f->hdc = CreateCompatibleDC(NULL);
  if (f->hdc == NULL) {
    free(f);
    return NULL;
  }

  SetMapperFlags(f->hdc, 1);
  SetMapMode(f->hdc, MM_TEXT);

  int isize = -MulDiv(size, GetDeviceCaps(f->hdc, LOGPIXELSY), 72);

  f->hfont = CreateFontA(isize, 0, 0, 0, fd->weight,
                         _gdi_font_slant(fd->slant), 0, 0, DEFAULT_CHARSET,
                         OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                         family);
  if (f->hfont == NULL) {
    DeleteDC(f->hdc);
    free(f);
    return NULL;
  }

  f->holdfont = (HFONT)SelectObject(f->hdc, (HGDIOBJ)f->hfont);
  if (f->holdfont == NULL) {
    DeleteObject(f->hfont);
    DeleteDC(f->hdc);
    free(f);
    return NULL;
  }

  return f;
}

void
gdi_font_destroy(
  gdi_font_t *f)
{
  assert(f != NULL);
  assert(f->hdc != NULL);
  assert(f->hfont != NULL);
  assert(f->holdfont != NULL);

  SelectObject(f->hdc, (HGDIOBJ)f->holdfont);
  DeleteObject(f->hfont);
  DeleteDC(f->hdc);
  free(f);
}

static double
_fixed_to_double(
  FIXED f)
{
  return (double)f.value + (double)f.fract / 65536.0;
}

bool
gdi_font_char_as_poly(
  const gdi_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(f != NULL);
  assert(f->hdc != NULL);
  assert(f->hfont != NULL);
  assert(t != NULL);
  assert(pen != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(c <= 0x10FFFF);

  static const MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };

  GLYPHMETRICS gm = { 0 };

  DWORD size = GetGlyphOutlineW(f->hdc, c, GGO_NATIVE, &gm, 0, NULL, &mat);
  if (size == GDI_ERROR) {
    return false;
  }

  char *data = (char *)calloc(size, sizeof(char));
  if (data == NULL) {
    return false;
  }

  DWORD res = GetGlyphOutlineW(f->hdc, c, GGO_NATIVE, &gm, size, data, &mat);
  if (res == GDI_ERROR) {
    free(data);
    return false;
  }

  const char *contour = data;

  while (contour < data + size) {

    const TTPOLYGONHEADER *header = (const TTPOLYGONHEADER *)contour;

    point_t cp, cp2, np;
    point_t lp = { pen->x + _fixed_to_double(header->pfxStart.x),
                   pen->y - _fixed_to_double(header->pfxStart.y) };
    transform_apply(t, &lp);
    polygon_add_point(p, lp);
    rect_expand(bbox, lp);

    const char *poly = contour + sizeof(TTPOLYGONHEADER);

    while (poly < contour + header->cb) {

      const TTPOLYCURVE *curve = (const TTPOLYCURVE *)poly;

      switch (curve->wType) {

        case TT_PRIM_LINE:
          for (int i = 0; i < curve->cpfx; ++i) {
            lp.x = pen->x + _fixed_to_double(curve->apfx[i].x);
            lp.y = pen->y - _fixed_to_double(curve->apfx[i].y);
            transform_apply(t, &lp);
            polygon_add_point(p, lp);
            rect_expand(bbox, lp);
          }
          break;

        case TT_PRIM_QSPLINE:
          for (int i = 0; i < curve->cpfx - 1; ++i) {
            cp.x = pen->x + _fixed_to_double(curve->apfx[i].x);
            cp.y = pen->y - _fixed_to_double(curve->apfx[i].y);
            np.x = pen->x + _fixed_to_double(curve->apfx[i+1].x);
            np.y = pen->y - _fixed_to_double(curve->apfx[i+1].y);
            if (i + 1 < curve->cpfx - 1) {
              np.x = (np.x + cp.x) / 2.0;
              np.y = (np.y + cp.y) / 2.0;
            }
            transform_apply(t, &cp);
            transform_apply(t, &np);
            quadratic_to_poly(lp, cp, np, p, 2);
            rect_expand(bbox, cp);
            rect_expand(bbox, np);
            lp = np;
          }
          break;

        case TT_PRIM_CSPLINE:
          for (int i = 0; i < curve->cpfx - 2; i += 3) {
            cp.x = pen->x + _fixed_to_double(curve->apfx[i].x);
            cp.y = pen->y - _fixed_to_double(curve->apfx[i].y);
            cp2.x = pen->x + _fixed_to_double(curve->apfx[i+1].x);
            cp2.y = pen->y - _fixed_to_double(curve->apfx[i+1].y);
            np.x = pen->x + _fixed_to_double(curve->apfx[i+2].x);
            np.y = pen->y - _fixed_to_double(curve->apfx[i+2].y);
            transform_apply(t, &cp);
            transform_apply(t, &cp2);
            transform_apply(t, &np);
            bezier_to_poly(lp, cp, cp2, np, p, 2);
            rect_expand(bbox, cp);
            rect_expand(bbox, cp2);
            rect_expand(bbox, np);
            lp = np;
          }
          break;

        default:
          assert(!"Bad curve type");
          break;
      }

      poly += sizeof(TTPOLYCURVE) + sizeof(POINTFX) * (curve->cpfx - 1);
    }

    polygon_end_subpoly(p, true);

    contour += header->cb;
  }

  pen->x += (double)gm.gmCellIncX;
  pen->y += (double)gm.gmCellIncY;

  free(data);

  return true;
}

#else

const int gdi_font = 0;

#endif /* HAS_GDI */
