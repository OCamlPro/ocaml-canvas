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
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "../config.h"
#include "../color.h"
#include "../context_internal.h"
#include "../hw_context_internal.h"
#include "gdi_target.h"

typedef struct gdi_hw_context_t {
  hw_context_t base;
} gdi_hw_context_t;

gdi_hw_context_t *
gdi_hw_context_create(
  gdi_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->hwnd != NULL);
  assert(width > 0);
  assert(height > 0);

  gdi_hw_context_t *context =
    (gdi_hw_context_t *)calloc(1, sizeof(gdi_hw_context_t));
  if (context == NULL) {
    return NULL;
  }

  context->base.base.width = width;
  context->base.base.height = height;

  return context;
}

void
gdi_hw_context_destroy(
  gdi_hw_context_t *context)
{
  assert(context != NULL);

  free(context);
}

bool
gdi_hw_context_resize(
  gdi_hw_context_t *context,
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
gdi_hw_context_present(
  gdi_hw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);

}

#else

const int gdi_hw_context = 0;

#endif /* HAS_GDI */
