/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_X11

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <xcb/xcb.h>

#include "../config.h"
#include "../color.h"
#include "../context_internal.h"
#include "../hw_context_internal.h"
#include "x11_backend_internal.h"
#include "x11_target.h"

typedef struct x11_hw_context_t {
  hw_context_t base;
  xcb_window_t wid;
} x11_hw_context_t;

x11_hw_context_t *
x11_hw_context_create(
  x11_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->wid != XCB_WINDOW_NONE);
  assert(width > 0);
  assert(height > 0);

  x11_hw_context_t *context =
    (x11_hw_context_t *)calloc(1, sizeof(x11_hw_context_t));
  if (context == NULL) {
    return NULL;
  }

  context->base.base.width = width;
  context->base.base.height = height;
  context->wid = target->wid;

  return context;
}

void
x11_hw_context_destroy(
  x11_hw_context_t *context)
{
  assert(context != NULL);

  free(context);
}

bool
x11_hw_context_resize(
  x11_hw_context_t *context,
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
x11_hw_context_present(
  x11_hw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->wid != XCB_WINDOW_NONE);

}

#else

const int x11_hw_context = 0;

#endif /* HAS_X11 */
