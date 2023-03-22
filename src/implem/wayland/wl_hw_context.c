/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <wayland-client.h>

#include "../config.h"
#include "../color.h"
#include "../context_internal.h"
#include "../hw_context_internal.h"
#include "wl_target.h"

typedef struct wl_hw_context_t {
  hw_context_t base;
  struct wl_surface *wl_surface;
} wl_hw_context_t;

wl_hw_context_t *
wl_hw_context_create(
  wl_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->wl_surface != NULL);
  assert(width > 0);
  assert(height > 0);

  wl_hw_context_t *context =
    (wl_hw_context_t *)calloc(1, sizeof(wl_hw_context_t));
  if (context == NULL) {
    return NULL;
  }

  context->base.base.width = width;
  context->base.base.height = height;
  context->wl_surface = target->wl_surface;

  return context;
}

void
wl_hw_context_destroy(
  wl_hw_context_t *context)
{
  assert(context != NULL);

  free(context);
}

bool
wl_hw_context_resize(
  wl_hw_context_t *context,
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
wl_hw_context_present(
  wl_hw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->wl_surface != NULL);

}

#else

const int wl_hw_context = 0;

#endif /* HAS_WAYLAND */
