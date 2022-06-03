/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#if defined HAS_X11 || defined HAS_WAYLAND

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../util.h"
#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../polygonize.h"
#include "../transform.h"
#include "../font_desc_internal.h"
#include "ft_font_internal.h"

static bool _ft_initialized = false;
static FcConfig *_fc_config = NULL;
static FT_Library _ft_library = NULL;

static void
_ft_font_ensure_init(
  void)
{
  if (_ft_initialized) {
    return;
  }

  int error = FT_Init_FreeType(&_ft_library);
  if (error != FT_Err_Ok) {
    return;
  }

  _fc_config = FcInitLoadConfigAndFonts();
  if (_fc_config == NULL) {
    FT_Done_FreeType(_ft_library);
    return;
  }

  _ft_initialized = true;
}

static int
_ft_font_slant(
  font_slant_t s)
{
  switch (s) {
    case SLANT_ROMAN: return FC_SLANT_ROMAN;
    case SLANT_ITALIC: return FC_SLANT_ITALIC;
    case SLANT_OBLIQUE: return FC_SLANT_OBLIQUE;
    default: assert(!"Invalid font slant"); return 0;
  }
}

static
int
_ft_font_weight(
  int32_t w)
{
  w = clip(w, 0, 1000);
  int32_t q = w / 100;
  int32_t r = w % 100;
  switch (q) {
    case 0: return 0;
    case 1: return 0 + r * 40 / 100;
    case 2: return 40 + r * 10 / 100;
    case 3: return 50 + r * 35 / 100;
    case 4: return 85 + r * 15 / 100;
    case 5: return 100 + r * 80 / 100;
    case 6: return 180 + r * 20 / 100;
    case 7: return 200 + r * 5 / 100;
    case 8: return 205 + r * 5 / 100;
    case 9: return 210 + r * 5 / 100;
    case 10: return 215;
    default: assert(!"Bad font weight");
  }
}

ft_font_t *
ft_font_create(
  font_desc_t *fd)
{
  assert(fd != NULL);
  assert(fd->weight >= 0);

  FcPattern *fp = NULL;
  FcPattern *fp_res = NULL;
  FT_Face face = NULL;

  const char *family = fd->family;
  if (family == NULL) {
    family = "Liberation Sans";
  }

  double size = fd->size;
  if (size <= 0.0) {
    size = 8.3;
  }

  // TODO: this should be part of backend init
  _ft_font_ensure_init();

  fp = FcPatternBuild(NULL,
                      FC_FAMILY, FcTypeString, (FcChar8 *)family,
                      FC_SLANT, FcTypeInteger, _ft_font_slant(fd->slant),
                      FC_WEIGHT, FcTypeInteger, _ft_font_weight(fd->weight),
                      FC_SIZE, FcTypeDouble, size,
                      (char *)0);
  if (fp == NULL) {
    goto error;
  }

  // Config-based substitutions
  FcBool bres = FcConfigSubstitute(_fc_config, fp, FcMatchPattern);
  if (bres == FcFalse) {
    goto error;
  }

  // Add default values for missing pattern elements
  // (style, weight, slant, size)
  FcDefaultSubstitute(fp);

  // Get the best matching font
  FcResult res;
  fp_res = FcFontMatch(_fc_config, fp, &res);
  if (fp_res == NULL) {
    goto error;
  }

  // Destroy no longer used objects
  FcPatternDestroy(fp);
  fp = NULL;

  FcValue file;
  res = FcPatternGet(fp_res, FC_FILE, 0, &file);
  if (res != FcResultMatch || file.type != FcTypeString) {
    goto error;
  }

  FcValue index;
  res = FcPatternGet(fp_res, FC_INDEX, 0, &index);
  if (res != FcResultMatch || index.type != FcTypeInteger) {
    goto error;
  }

  int error =
    FT_New_Face(_ft_library, (const char *)file.u.s, index.u.i, &face);
  if (error != FT_Err_Ok) {
    goto error;
  }

  // Destroy no longer used objects
  FcPatternDestroy(fp_res);
  fp_res = NULL;

  error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
  if (error != FT_Err_Ok) {
    goto error;
  }

  //FT_Set_Char_Size(face, 0, size * 64.0, 72, 72);
  //FT_Set_Char_Size(face, 0, size * 26.6, 72, 72);
  FT_Set_Pixel_Sizes(face, 0, size * 96.0 / 72.0);
  if (error != FT_Err_Ok) {
    goto error;
  }

  ft_font_t *f = (ft_font_t *)calloc(1, sizeof(ft_font_t));
  if (f == NULL) {
    goto error;
  }

  f->ft_face = face;

  return f;

error:
  if (face != NULL) {
    FT_Done_Face(face);
  }
  if (fp_res != NULL) {
    FcPatternDestroy(fp_res);
  }
  if (fp != NULL) {
    FcPatternDestroy(fp);
  }
  return NULL;
}

void
ft_font_destroy(
  ft_font_t *f)
{
  assert(f != NULL);
  assert(f->ft_face != NULL);

  FT_Done_Face(f->ft_face);
  free(f);
}

bool
ft_font_char_as_poly(
  const ft_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(f != NULL);
  assert(f->ft_face != NULL);
  assert(t != NULL);
  assert(pen != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(c <= 0x10FFFF);

/*
  double a, b, c, d;
  transform_extract_ft(t, &a, &b, &c, &d);
//  FT_Matrix m = { a * 64.0, b * 64.0, c * 64.0, d * 64.0 };
  FT_Matrix m = { 16.0, 16.0, 16.0, 16.0 };
//  FT_Vector ppen = { pen->x * 64.0, pen->y * 64.0 };
//  FT_Vector ppen = { 0.0, 0.0 };
  FT_Set_Transform(_ft_face, &m, NULL / * &ppen * // *);
*/

  int error = FT_Load_Char(f->ft_face, c, FT_LOAD_NO_BITMAP);
  if (error) {
    return false;
  }

  FT_GlyphSlot glyph = f->ft_face->glyph;

  if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
    return false;
  }

  for (int ic = 0; ic < glyph->outline.n_contours; ++ic) {

    int itp; // temporary point
    int ilp = glyph->outline.contours[ic]; // last point
    int ifp = (ic == 0) ? 0 : glyph->outline.contours[ic-1]+1; // first point
    int ip = (ic == 0) ? 0 : ifp; // current point

    point_t cp, cp2, np;
    point_t lp = { pen->x + glyph->outline.points[ilp].x / 64.0,
                   pen->y - glyph->outline.points[ilp].y / 64.0 };

    if ((glyph->outline.tags[ifp] & 0x03) == 0x00) {
      if ((glyph->outline.tags[ilp] & 0x03) == 0x00) {
        lp.x = (lp.x + pen->x + glyph->outline.points[ifp].x / 64.0) / 2.0;
        lp.y = (lp.y + pen->y - glyph->outline.points[ifp].y / 64.0) / 2.0;
      }
    }

    transform_apply(t, &lp);

    while (ip <= ilp) {

      switch (glyph->outline.tags[ip] & 0x03) {

        case 0x01: // Point is on the curve
        case 0x03: // should not happen
          lp.x = pen->x + glyph->outline.points[ip].x / 64.0;
          lp.y = pen->y - glyph->outline.points[ip].y / 64.0;
          transform_apply(t, &lp);
          polygon_add_point(p, lp);
          rect_expand(bbox, lp);
          ++ip;
          break;

        case 0x00: // Point is off the curve, second-order control point
          cp.x = pen->x + glyph->outline.points[ip].x / 64.0;
          cp.y = pen->y - glyph->outline.points[ip].y / 64.0;
          itp = ip < ilp ? ip + 1 : ifp;
          np.x = pen->x + glyph->outline.points[itp].x / 64.0;
          np.y = pen->y - glyph->outline.points[itp].y / 64.0;
          if ((glyph->outline.tags[itp] & 0x03) == 0x00) {
            np.x = (np.x + cp.x) / 2.0;
            np.y = (np.y + cp.y) / 2.0;
          } else {
            ++ip;
          }
          transform_apply(t, &cp);
          transform_apply(t, &np);
          quadratic_to_poly(lp, cp, np, p, 2);
          rect_expand(bbox, cp);
          rect_expand(bbox, np);
          lp = np;
          ++ip;
          break;

        case 0x02: // Point is off the curve, third-order control point
          cp.x = pen->x + glyph->outline.points[ip].x / 64.0;
          cp.y = pen->y - glyph->outline.points[ip].y / 64.0;
          itp = ip < ilp ? ip + 1 : ifp;
          cp2.x = pen->x + glyph->outline.points[itp].x / 64.0;
          cp2.y = pen->y - glyph->outline.points[itp].y / 64.0;
          itp = ip + 1 < ilp ? ip + 2 : ifp;
          np.x = pen->x + glyph->outline.points[itp].x / 64.0;
          np.y = pen->y - glyph->outline.points[itp].y / 64.0;
          transform_apply(t, &cp);
          transform_apply(t, &cp2);
          transform_apply(t, &np);
          bezier_to_poly(lp, cp, cp2, np, p, 2);
          rect_expand(bbox, cp);
          rect_expand(bbox, cp2);
          rect_expand(bbox, np);
          lp = np;
          ip += 3;
          break;

        default:
          assert(!"Bad point tag");
          break;
      }
    }

    polygon_end_subpoly(p, true);
  }

  pen->x += f->ft_face->glyph->advance.x / 64.0;
  pen->y += f->ft_face->glyph->advance.y / 64.0;

  return true;
}

/*
on-curve
quadratic control points (also known as 'conic')
cubic control points

The quadratic control points are grouped with on-curve points on either side
of them to form the three points needed to define a quadratic Bezier spline.

The cubic control points must occur in pairs, and are grouped with on-curve
points on either side to make up the four points needed for a cubic Bezier
spline.

However, there is a short-hand notation for quadratic points only. Where
quadratic points occur next to each other, an on-curve control point is
interpolated between them.

And there is another convention, that if a closed path starts with a quadratic
point, the last point of the path is examined, and if it is quadratic, an
on-curve point is interpolated between them, and the path is taken to start
with that on-curve point; if the last point is not a quadratic control point,
it is itself used for the start point.
*/

#else

const int ft_font = 0;

#endif /* HAS_X11 || HAS_WAYLAND */
