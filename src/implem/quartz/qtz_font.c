/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_QUARTZ

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <Cocoa/Cocoa.h>
#include <Foundation/NSDictionary.h>
#include <CoreText/CoreText.h>

#include "../point.h"
#include "../rect.h"
#include "../polygon.h"
#include "../polygonize.h"
#include "../transform.h"
#include "../font_desc_internal.h"
#include "qtz_font_internal.h"

#define ALLOC_POOL NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE_POOL [pool release]

static double
_qtz_font_slant(
  font_slant_t s)
{
  // -1.0 to 0.0, 1.0 = 30 degrees
  switch (s) {
    case SLANT_ROMAN: return 0.0;
    case SLANT_ITALIC: return 1.0;
    case SLANT_OBLIQUE: return 1.0; /* Approximate */
    default: assert(!"Invalid font slant"); return 0;
  }
}

static double
_qtz_font_weight(
  int32_t s)
{
  //-1.0 to 1.0, 0.0 = normal
  return 1.0;
}



qtz_font_t *
qtz_font_create(
  font_desc_t *fd)
{
  assert(fd != NULL);
  assert(fd->weight >= 0);

  const char *family = fd->family;
  if (family == NULL) {
    family = "San Francisco";
  }

  double size = fd->size;
  if (size <= 0.0) {
    size = 8.3;
  }

  qtz_font_t *f = (qtz_font_t *)calloc(1, sizeof(qtz_font_t));
  if (f == NULL) {
    return NULL;
  }

  ALLOC_POOL;

  NSDictionary *attrib =
    [[NSDictionary alloc] initWithObjectsAndKeys:
     [NSString stringWithUTF8String:family], (id)kCTFontFamilyNameAttribute,
     [NSNumber numberWithFloat:size],        (id)kCTFontSizeAttribute,
     [[NSDictionary alloc] initWithObjectsAndKeys:
      [NSNumber numberWithFloat:_qtz_font_weight(fd->weight)],
      (id)kCTFontWeightTrait,
      [NSNumber numberWithFloat:_qtz_font_slant(fd->slant)],
      (id)kCTFontSlantTrait,
      nil
     ], (id)kCTFontTraitsAttribute,
     nil];

  CTFontDescriptorRef desc =
    CTFontDescriptorCreateWithAttributes((CFDictionaryRef)attrib);
  // release attrib ?

  CTFontRef font = CTFontCreateWithFontDescriptor(desc, 0.0, NULL);

  CFRelease(desc);

  RELEASE_POOL;

  f->font = font;

  return f;
}

void
qtz_font_destroy(
  qtz_font_t *f)
{
  assert(f != NULL);
  assert(f->font != NULL);

  CFRelease(f->font);
  free(f);
}

typedef struct font_path_info_t {
  const transform_t *t;
  point_t *pen;
  polygon_t *p;
  rect_t *bbox;
  point_t fp;
  point_t lp;
} font_path_info_t;

static void
_qtz_font_path_apply(
  void *info,
  const CGPathElement *e)
{
  font_path_info_t *pi = (font_path_info_t *)info;
  point_t cp, cp2, np;

  switch (e->type) {

    case kCGPathElementMoveToPoint:
      np.x = pi->pen->x + e->points[0].x;
      np.y = pi->pen->y - e->points[0].y;
      transform_apply(pi->t, &np);
      polygon_end_subpoly(pi->p, false);
      polygon_add_point(pi->p, np);
      rect_expand(pi->bbox, np);
      pi->lp = pi->fp = np;
      break;

    case kCGPathElementAddLineToPoint:
      np.x = pi->pen->x + e->points[0].x;
      np.y = pi->pen->y - e->points[0].y;
      transform_apply(pi->t, &np);
      polygon_add_point(pi->p, np);
      rect_expand(pi->bbox, np);
      pi->lp = np;
      break;

    case kCGPathElementAddQuadCurveToPoint:
      cp.x = pi->pen->x + e->points[0].x;
      cp.y = pi->pen->y - e->points[0].y;
      np.x = pi->pen->x + e->points[1].x;
      np.y = pi->pen->y - e->points[1].y;
      transform_apply(pi->t, &cp);
      transform_apply(pi->t, &np);
      quadratic_to_poly(pi->lp, cp, np, pi->p, 2);
      rect_expand(pi->bbox, cp);
      rect_expand(pi->bbox, np);
      pi->lp = np;
      break;

    case kCGPathElementAddCurveToPoint:
      cp.x = pi->pen->x + e->points[0].x;
      cp.y = pi->pen->y - e->points[0].y;
      cp2.x = pi->pen->x + e->points[1].x;
      cp2.y = pi->pen->y - e->points[1].y;
      np.x = pi->pen->x + e->points[2].x;
      np.y = pi->pen->y - e->points[2].y;
      transform_apply(pi->t, &cp);
      transform_apply(pi->t, &cp2);
      transform_apply(pi->t, &np);
      bezier_to_poly(pi->lp, cp, cp2, np, pi->p, 2);
      rect_expand(pi->bbox, cp);
      rect_expand(pi->bbox, cp2);
      rect_expand(pi->bbox, np);
      pi->lp = np;
      break;

    case kCGPathElementCloseSubpath:
      polygon_end_subpoly(pi->p, true);
      pi->lp = pi->fp;
      break;

    default:
      assert(!"Bad path element type");
      break;
  }
}

bool
qtz_font_char_as_poly(
  const qtz_font_t *f,
  const transform_t *t,
  uint32_t c,
  point_t *pen, // in/out
  polygon_t *p, // out
  rect_t *bbox) // out
{
  assert(f != NULL);
  assert(f->font != NULL);
  assert(t != NULL);
  assert(pen != NULL);
  assert(p != NULL);
  assert(bbox != NULL);
  assert(c <= 0x10FFFF);

  int nb_chars = 0;
  UniChar uc[2] = { 0 };
  if (c < 0x10000) {
    nb_chars = 1;
    uc[0] = (UniChar)c;
  } else {
    nb_chars = 2;
    uc[0] = (UniChar)(0xD800 + ((c - 0x10000) >> 10));
    uc[1] = (UniChar)(0xDC00 + (c & 0x3FF));
  }
  CGGlyph glyph[2] = { 0 };

  bool res = CTFontGetGlyphsForCharacters(f->font, uc, glyph, nb_chars);
  if (res == false) {
    return false;
  }

  CGPathRef path = CTFontCreatePathForGlyph(f->font, glyph[0], NULL);

  // Note: some valid chars have no path (eg: space, bitmap chars, etc...)
  if (path != NULL) {

    font_path_info_t pi = { t, pen, p, bbox,
                            point(0.0, 0.0), point(0.0, 0.0) };

    CGPathApply(path, (void *)&pi, _qtz_font_path_apply);
    CGPathRelease(path);
  }

  CGSize advance;
  CTFontGetAdvancesForGlyphs(f->font, kCTFontOrientationDefault,
                             glyph, &advance, 1);
  pen->x += (double)advance.width;
  pen->y += (double)advance.height;

  return true;
}

#else

const int qtz_font = 0;

#endif /* HAS_QUARTZ */
