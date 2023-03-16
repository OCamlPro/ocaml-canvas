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
#include "qtz_util.h"
#include "qtz_backend.h"
#include "qtz_target.h"
#include "qtz_context.h"

@interface CanvasView : NSView
{
  @public context_impl_qtz_t *impl;
}
@end

typedef struct context_impl_qtz_t {
  impl_type_t type;
  CGContextRef ctxt;
  CanvasView *nsview;
} context_impl_qtz_t;

void
context_present_qtz_impl2(
  context_impl_qtz_t *impl,
  int32_t width,
  int32_t height,
  qtz_present_data_t *present_data);

@implementation CanvasView { }

- (BOOL)isFlipped
{
  return FALSE;
}

- (BOOL)preservesContentDuringLiveResize
{
  return TRUE;
}

- (void)drawRect:(NSRect)rect // called after setNeedsDisplay:YES
{
/*
  qtz_window_t *w = qtz_backend_get_window([self window]);
  if (w != NULL) {
    event_t evt;
    evt.type = EVENT_PRESENT;
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    event_notify(qtz_back->listener, &evt);
  }
*/
  qtz_present_data_t present_data = { 0 };

  context_present_qtz_impl2(self->impl, 0, 0, &present_data);

}

@end

static CGContextRef
_context_create_qtz_bitmap_context(
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

context_impl_qtz_t *
context_create_qtz_impl(
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

  context_impl_qtz_t *impl =
    (context_impl_qtz_t *)calloc(1, sizeof(context_impl_qtz_t));
  if (impl == NULL) {
    return NULL;
  }

  ALLOC_POOL; // don't know if necessary here

  CanvasView *nsview =
    [[CanvasView alloc] initWithFrame:[target->nswin frame]];
  [nsview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [target->nswin setContentView:nsview];
  nsview->impl = impl;

  RELEASE_POOL;

  CGContextRef ctxt =
    _context_create_qtz_bitmap_context(width, height, data);
  if (ctxt == NULL) {
    free(impl);
    return NULL;
  }

  impl->type = IMPL_QUARTZ;
  impl->ctxt = ctxt;
  impl->nsview = nsview;

  return impl;
}

void
context_destroy_qtz_impl(
  context_impl_qtz_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_QUARTZ);

  if (impl->ctxt != NULL) {
    // Note: does not free the underlying memory if explicitly given
    CGContextRelease(impl->ctxt);
  }

  if (impl->nsview != NULL) {
    [impl->nsview release];
  }

}

static void
_raw_context_copy(
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
context_resize_qtz_impl(
  context_impl_qtz_t *impl,
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
    _context_create_qtz_bitmap_context(d_width, d_height, d_data);
  if (ctxt == NULL) {
    return false;
  }

  _raw_context_copy(*s_data, s_width, s_height, *d_data, d_width, d_height);

  if (impl->ctxt) {
    CGContextRelease(impl->ctxt);
  }

  impl->ctxt = ctxt;

  return true;
}

void
context_present_qtz_impl(
  context_impl_qtz_t *impl,
  int32_t width,
  int32_t height,
  qtz_present_data_t *present_data)
{
  // TODO
}

void
context_present_qtz_impl2(
  context_impl_qtz_t *impl,
  int32_t width,
  int32_t height,
  qtz_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height > 0);

  CGContextRef ctxt = [[NSGraphicsContext currentContext] CGContext];
  // CGContext on 10.10 and above, graphicsPort on 10.13 and below

  CGContextSaveGState(ctxt);

  // The underlying memory is copy-on write, so we should just create
  // this object before blitting and then release it immediately
  CGImageRef img = CGBitmapContextCreateImage(impl->ctxt);
height = CGImageGetHeight(img);
width = CGImageGetWidth(img);
  CGContextDrawImage(ctxt, CGRectMake(0, impl->nsview.frame.size.height - height, width, height), img);

  [[NSGraphicsContext currentContext] flushGraphics];

  CGImageRelease(img);

  CGContextRestoreGState(ctxt);
}

#else

const int qtz_context = 0;

#endif /* HAS_QUARTZ */
