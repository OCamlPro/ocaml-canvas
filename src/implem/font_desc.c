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
#include <assert.h>

#include "unicode.h"
#include "font_desc.h"
#include "font_desc_internal.h"

#define MAX_FAMILY_SIZE 256

font_desc_t *
font_desc_create(
  void)
{
  font_desc_t *fd = (font_desc_t *)calloc(1, sizeof(font_desc_t));
  if (fd == NULL) {
    return NULL;
  }

  font_desc_reset(fd);

  return fd;
}

void
font_desc_destroy(
  font_desc_t *fd)
{
  assert(fd != NULL);

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  free(fd);
}

void
font_desc_reset(
  font_desc_t *fd)
{
  assert(fd != NULL);

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  fd->family = NULL;
  fd->size = 0.0;
  fd->slant = SLANT_ROMAN;
  fd->weight = 400.0;
  fd->scale = 1.0;
}

font_desc_t *
font_desc_copy(
  const font_desc_t *fd)
{
  assert(fd != NULL);

  font_desc_t *fdc = (font_desc_t *)calloc(1, sizeof(font_desc_t));
  if (fdc == NULL) {
    return NULL;
  }

  if (fd->family != NULL) {
    fdc->family = strndup(fd->family, MAX_FAMILY_SIZE);
    if (fdc->family == NULL) {
      font_desc_destroy(fdc);
      return NULL;
    }
  } else {
    fdc->family = NULL;
  }

  fdc->size = fd->size;
  fdc->slant = fd->slant;
  fdc->weight = fd->weight;
  fdc->scale = fd->scale;

  return fdc;
}

bool
font_desc_equal(
  const font_desc_t *fd1,
  const font_desc_t *fd2)
{
  assert(fd1 != NULL);
  assert(fd2 != NULL);

  return
    ((fd1->family != NULL && fd2->family != NULL &&
      strncmp(fd1->family, fd2->family, MAX_FAMILY_SIZE) == 0) ||
     (fd1->family == NULL && fd2->family == NULL)) &&
    fd1->size == fd2->size &&
    fd1->slant == fd2->slant &&
    fd1->weight == fd2->weight &&
    fd1->scale == fd2->scale;
}

bool
font_desc_is_set(
  const font_desc_t *fd)
{
  assert (fd != NULL);

  return fd->family != NULL;
}

bool
font_desc_set(
  font_desc_t *fd,
  const char *family,
  double size,
  font_slant_t slant,
  int32_t weight)
{
  assert(fd != NULL);
  assert(family != NULL);
  assert(size > 0.0);
  assert(weight >= 0);

  family = strndup(family, MAX_FAMILY_SIZE);
  if (family == NULL) {
    return false;
  }

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  fd->family = family;
  fd->size = size;
  fd->slant = slant;
  fd->weight = weight;
  fd->scale = 1.0;

  return true;
}

void
font_desc_scale(
  font_desc_t *fd,
  double s)
{
  assert(fd != NULL);

  fd->scale = s;
}


/*
canvas_(transform) :
 -> compute the scaled font size
 -> if "created" is false, just store the new scaled font size
 -> if "created" is true and new scaled font size != old scaled font
    size, set "created" to false and store the new scaled font size
 --> font_update() // scale only
*/

/*
canvas_fill_text: // + use CreateCompatibleDC, GetTextMetrics, GetGlyphOutline
 -> if "created" is true, just draw using the current font (Win32: SelectObject)
 -> if "created" is false
    -> unload the existing font, if present
    -> load the new font
       * Win32: use CreateFont
       * Fc/Ft: locate font + FT_New_Face + FT_Set_Pixel_Sizes
 --> font_load() function (or just font get_glyph ?)
*/


/*

JS
cm
mm
inch (1in = 96 px = 2.54 px)
px (1px = 1/96 of 1 in) (default is 16) (depends on DPI)
pt (1pt = 1/72 of 1 in) (72 = 24 * 3) (96 = 24 * 4)    DTP (desktop publishing point) = 1/72 inch
pc (1 pc = 12 pt)

Win32
height in logical units...

OSX
size in points (float), default is 12.0

FreeType
uses 26.6 fractional points
or pixels
* pixel_size = (size in pt) * resolution / 72
           (because as in JS, pt 1pt = 1/72 of 1 in)

Use points as floats

JS: directly use point size
Win32: ???
OSX: directly use point size
FT: pixel size = point size * DPI / 72 (if DPI = 72, then pixel size = point size)

Could have > 0 for point size and < for pixel sizes

72 = 62 pixs

96 DPI
*/


  /* font
       font-family (serif/sans-serif/monospace/cursive/NAME)
         cairo: font_family
         ft: - use fontconfig
         win: pszFaceName
         osx: name
         java: name (as String)
       font-size (small/medium/large/.../XX/XX%)
         cairo: font_size (as double)
         ft: use FT_Set_Char_Size / FT_Set_Pixel_Size
         win: cWidth/wHeight (maybe can use width for stretch) (+/- values)
         osx: size
         java: size (as int)
       font-style (normal/italic/oblique/oblique XXdeg)
         note1: italic is builtin the font, oblique is a skew transform
         note2: HTML canvas do not have underline/strikeout
         cairo: font_slant (normal, italic, oblique)
         ft: - use fontconfig
         win: bItalic (bUnderline, bStrikeOut)
         osx: CTFontSymbolicTraits (kCTFontTraitItalic + Bold + ...)
         java: plain/italic + bold + attributes (underline/strikethrough)
       font-weight (normal=400/bold=700/lighter/bolder/XX)
         note (js): font must support different weights
         cairo: font_weight
         ft: - use fontconfig
         win: cWeight (as integer, 700 = bold, 0 = don't care/default)
         osx: kCTFontTraitsAttribute + kCTFontWeightTrait
         java: attribute (weight)

       font-stretch (normal/condensed/expanded/XX%)
         cairo: -
         win: use width ? (can't use EXTLOGFONT/PANOSE ?)
         osx: CTFontSymbolicTraits (kCTFontTraitCondensed/Expanded)
         java: use width attribute ?

       font-variant (normal/none/stylistic/swash/small-caps/........)
         note: minor variations
         win: iPitchAndFamily? (seems no)
         osx: CTFontStylisticClass (kCTFontClassModernSerifs/.....)
         java: use attributes

       line-height (normal/XX/XX/XX%) (not useful on canvas ?)

     textAlign (left/right/center/start+/end)
     textBaseline (top/middle/bottom/hanging/middle/alphabetic+/ideographic)

     javascript standard fonts:
       Serif, SansSerif, Monospaced, Dialog (= SansSerif), DialogInput (=Monosp)
     cairo standard fonts:
       serif, sans-serif, monospaces, cursive, fantasy
   */
  // use max_width to adjust font if needed


