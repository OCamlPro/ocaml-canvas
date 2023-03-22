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
#include "../context_internal.h"
#include "../sw_context_internal.h"
#include "qtz_sw_context.h"
#include "qtz_target.h"
#include "qtz_util.h"

@class CanvasView;

typedef struct qtz_sw_context_t {
  sw_context_t base;
  CGContextRef ctxt;
  CanvasView *nsview;
} qtz_sw_context_t;

@interface CanvasView : NSView
{
  @public qtz_sw_context_t *context;
}
@end

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
    evt.type = EVENT_PRESENT; // not needed
    evt.time = qtz_get_time();
    evt.target = (void *)w;
    event_notify(qtz_back->listener, &evt);
  }
*/

  qtz_sw_context_present(self->context);

}

@end

static CGContextRef
_qtz_sw_context_create_bitmap_context(
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(width > 0);
  assert(height > 0);
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

qtz_sw_context_t *
qtz_sw_context_create(
  qtz_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->nswin != NULL);
  assert(width > 0);
  assert(height > 0);

  qtz_sw_context_t *context =
    (qtz_sw_context_t *)calloc(1, sizeof(qtz_sw_context_t));
  if (context == NULL) {
    return NULL;
  }

  color_t_ *data = NULL;
  CGContextRef ctxt =
    _qtz_sw_context_create_bitmap_context(width, height, &data);
  if (ctxt == NULL) {
    assert(data == NULL);
    free(context);
    return NULL;
  }
  assert(data != NULL);

  ALLOC_POOL; // don't know if necessary here

  CanvasView *nsview =
    [[CanvasView alloc] initWithFrame:[target->nswin frame]];
  [nsview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [target->nswin setContentView:nsview];
  nsview->context = context;

  RELEASE_POOL;

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = data;
  context->ctxt = ctxt;
  context->nsview = nsview;

  return context;
}

void
qtz_sw_context_destroy(
  qtz_sw_context_t *context)
{
  assert(context != NULL);

  if (context->ctxt != NULL) {
    CGContextRelease(context->ctxt);
    free(context->base.data); /* We allocated it, we have to free it */
  }

  if (context->nsview != NULL) {
//    [[context->nsview window] setContentView:nil];
    [context->nsview release];
  }

}

bool
qtz_sw_context_resize(
  qtz_sw_context_t *context,
  int32_t width,
  int32_t height)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->base.data != NULL);
  assert(context->ctxt != NULL);
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = NULL;
  CGContextRef ctxt =
    _qtz_sw_context_create_bitmap_context(width, height, &data);
  if (ctxt == NULL) {
    assert(data == NULL);
    return false;
  }
  assert(data != NULL);

  _sw_context_copy_to_buffer(&context->base, data, width, height);

  CGContextRelease(context->ctxt);

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = data;
  context->ctxt = ctxt;

  return true;
}

void
qtz_sw_context_present(
  qtz_sw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->ctxt != NULL);
  assert(context->nsview != NULL);

//+ (NSGraphicsContext *)graphicsContextWithWindow:(NSWindow *)window;
//to attach a drawing context to a window

  // CGContext on 10.10 and above, graphicsPort on 10.13 and below
  CGContextRef ctxt = [[NSGraphicsContext currentContext] CGContext];

  CGContextSaveGState(ctxt);

  // The underlying memory is copy-on write, so we should just create
  // this object before blitting and then release it immediately
  CGImageRef img = CGBitmapContextCreateImage(context->ctxt);

  CGRect rect =
    CGRectMake(0, context->nsview.frame.size.height - context->base.base.height,
               context->base.base.width, context->base.base.height);
  CGContextDrawImage(ctxt, rect, img);

  [[NSGraphicsContext currentContext] flushGraphics];

  CGImageRelease(img);

  CGContextRestoreGState(ctxt);
}

#else

const int qtz_sw_context = 0;

#endif /* HAS_QUARTZ */
