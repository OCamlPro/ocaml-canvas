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
#include "../hw_context_internal.h"
#include "qtz_hw_context.h"
#include "qtz_target.h"
#include "qtz_util.h"

@class CanvasHWView;

typedef struct qtz_hw_context_t {
  hw_context_t base;
  CanvasHWView *nsview;
} qtz_hw_context_t;

@interface CanvasHWView : NSView
{
  @public qtz_hw_context_t *context;
}
@end

@implementation CanvasHWView { }

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

  qtz_hw_context_present(self->context);

}

@end

qtz_hw_context_t *
qtz_hw_context_create(
  qtz_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->nhwin != NULL);
  assert(width > 0);
  assert(height > 0);

  qtz_hw_context_t *context =
    (qtz_hw_context_t *)calloc(1, sizeof(qtz_hw_context_t));
  if (context == NULL) {
    return NULL;
  }

  ALLOC_POOL; // don't know if necessary here

  CanvasHWView *nsview =
    [[CanvasHWView alloc] initWithFrame:[target->nswin frame]];
  [nsview setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [target->nswin setContentView:nsview];
  nsview->context = context;

  RELEASE_POOL;

  context->base.base.width = width;
  context->base.base.height = height;
  context->nsview = nsview;

  return context;
}

void
qtz_hw_context_destroy(
  qtz_hw_context_t *context)
{
  assert(context != NULL);

  if (context->nsview != NULL) {
//    [[context->nsview window] setContentView:nil];
    [context->nsview release];
  }

}

bool
qtz_hw_context_resize(
  qtz_hw_context_t *context,
  int32_t width,
  int32_t height)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(width > 0);
  assert(height > 0);

  context->base.base.width = width;
  context->base.base.height = height;

  return true;
}

void
qtz_hw_context_present(
  qtz_hw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->nsview != NULL);

}

#else

const int qtz_hw_context = 0;

#endif /* HAS_QUARTZ */
