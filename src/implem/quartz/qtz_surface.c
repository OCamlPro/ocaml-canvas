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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <Cocoa/Cocoa.h>

#include "../config.h"
#include "../color.h"
#include "qtz_backend.h"
#include "qtz_window.h"
#include "qtz_target.h"

typedef struct surface_impl_qtz_t {
  impl_type_t type;
  CGContextRef ctxt;
  NSView *nsview;
} surface_impl_qtz_t;

static CGContextRef
_surface_create_qtz_bitmap_context(
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);

  *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
  if (*data == NULL) {
    return NULL;
  }

  CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
  if (cs == NULL) {
    free(*data);
    *data = NULL;
    return NULL;
  }

  CGContextRef ctxt =
    CGBitmapContextCreate((void *)*data, width, height, 8, width * 4, cs,
                          kCGImageAlphaPremultipliedFirst |
                          kCGBitmapByteOrder32Little);
  if (ctxt == NULL) {
    CGColorSpaceRelease(cs);
    free(*data);
    *data = NULL;
    return NULL;
  }

  CGColorSpaceRelease(cs);

  return ctxt;
}

surface_impl_qtz_t *
surface_create_qtz_impl(
  qtz_target_t *target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(target != NULL);
  assert(target->nsview != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  surface_impl_qtz_t *impl =
    (surface_impl_qtz_t *)calloc(1, sizeof(surface_impl_qtz_t));
  if (impl == NULL) {
    return NULL;
  }

  CGContextRef ctxt = _surface_create_qtz_bitmap_context(width, height, data);
  if (ctxt == NULL) {
    free(impl);
    return NULL;
  }

  /* Flip vertical coordinates, so origin is at the top-left */
  //CGContextConcatCTM(ctxt, CGAffineTransformMake(1, 1, 0, -1, 0, height));

  impl->type = IMPL_QUARTZ;
  impl->nsview = target->nsview;
  impl->ctxt = ctxt;

  return impl;
}


void
surface_destroy_qtz_impl(
  surface_impl_qtz_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_QUARTZ);

  if (impl->ctxt) {
    CGContextRelease(impl->ctxt); // Does it affect s->data ?
  }
}

static void
_raw_surface_copy(
  color_t_ *s_data,
  int32_t s_width,
  int32_t s_height,
  color_t_ *d_data,
  int32_t d_width,
  int32_t d_height)
{
  assert(s_data != NULL);
  assert(s_width > 0);
  assert(s_height > 0);
  assert(d_data != NULL);
  assert(d_width > 0);
  assert(d_height > 0);
  uint32_t min_width = d_width < s_width ? d_width : s_width;
  uint32_t min_height = d_height < s_height ? d_height : s_height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      d_data[i * d_width + j] = s_data[i * s_width + j];
    }
  }
}

bool
surface_resize_qtz_impl(
  surface_impl_qtz_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  assert(impl != NULL);
  assert(s_width > 0);
  assert(s_height  > 0);
  assert(s_data != NULL);
  assert(*s_data != NULL);
  assert(d_width > 0);
  assert(d_height  > 0);
  assert(d_data != NULL);
  assert(*d_data == NULL);

  CGContextRef ctxt =
    _surface_create_qtz_bitmap_context(d_width, d_height, d_data);
  if (ctxt == NULL) {
    return false;
  }

  _raw_surface_copy(*s_data, s_width, s_height, *d_data, d_width, d_height);

  if (impl->ctxt) {
    CGContextRelease(impl->ctxt);
  }

  impl->ctxt = ctxt;

  return true;
}

void
surface_present_qtz_impl(
  surface_impl_qtz_t *impl,
  int32_t width,
  int32_t height,
  qtz_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height  > 0);

  if (present_data->use_lock) {
    if (![impl->nsview lockFocusIfCanDraw])
      printf("Can't lock\n");
  }

  CGContextRef ctxt = [[NSGraphicsContext currentContext] graphicsPort];


  CGContextSaveGState(ctxt);

  //CGAffineTransform at = CGContextGetCTM(ctxt);
  //printf("Transf: %f, %f, %f, %f, %f, %f\n", at.a, at.b, at.c, at.d, at.tx, at.ty);

  /* Flip vertical coordinates, so origin is at the top-left */
  //CGContextConcatCTM(impl->ctxt, CGAffineTransformMake(1, 0, 0, -1, 0, height));



//static CGFloat colors[4] = { 1.0, 1.0, 1.0, 1.0 };
/*
if (colors[0] == 0.0) { colors[0] = 1.0; colors[1] = 0.0; }
else if (colors[1] == 0.0) { colors[1] = 1.0; colors[2] = 0.0; }
else if (colors[2] == 0.0) { colors[2] = 1.0; colors[0] = 0.0; }
*/
/*
CGContextSetLineWidth(impl->ctxt, 4.0);
CGContextSetStrokeColor(impl->ctxt, colors);
CGContextSetFillColor(impl->ctxt, colors);
CGContextMoveToPoint(impl->ctxt, 10, 40);
CGContextAddLineToPoint(impl->ctxt, 50, 20);
CGContextDrawPath(impl->ctxt, kCGPathStroke);
*/
/*
CGFontRef font = CGFontCreateWithFontName(CFSTR("Arial"));
if (font == NULL) printf("Font failure\n");
CGContextSetFont(impl->ctxt, font);
CGContextSetFontSize(impl->ctxt, 12.0);*//*
CGContextSelectFont(impl->ctxt, "Arial", 40, kCGEncodingMacRoman);
//CGContextSetTextPosition(impl->ctxt, 50, 50);
CGContextSetTextDrawingMode(impl->ctxt, kCGTextFill);
CGContextSetTextMatrix(impl->ctxt, CGAffineTransformIdentity);
CGContextShowTextAtPoint(impl->ctxt, 50, 50, "Test", 4);
*/

  // the underlying memory is copy-on write, so we should
  // just create this object before blitting and then
  // release it immediately

  CGImageRef img = CGBitmapContextCreateImage(impl->ctxt);
  CGContextDrawImage(ctxt, CGRectMake(0, impl->nsview.frame.size.height - height, width, height), img);
  CGImageRelease(img);

  CGContextRestoreGState(ctxt);

  [[NSGraphicsContext currentContext] flushGraphics];

  if (present_data->use_lock) {
    [impl->nsview unlockFocus];
  }


/*
  NSGraphicsContext *cc = [NSGraphicsContext currentContext];

  NSGraphicsContext *wc = [NSGraphicsContext graphicsContextWithWindow:[impl->nsview window]];
  [NSGraphicsContext setCurrentContext:wc];

  CGContextRef ctxt = [wc CGContext];

  // the underlying memory is copy-on write, so we should
  // just create this object before blitting and then
  // release it immediately
  CGImageRef img = CGBitmapContextCreateImage(impl->ctxt);
  CGContextDrawImage(ctxt, CGRectMake(0, 0, width, height), img);
  CGImageRelease(img);

  [wc flushGraphics];

  [NSGraphicsContext setCurrentContext:cc];
*/
}

#else

const int qtz_surface = 0;

#endif /* HAS_QUARTZ */
