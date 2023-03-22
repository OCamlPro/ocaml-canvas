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

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "../config.h"
#include "../context_internal.h"
#include "../hw_context_internal.h"
#include "wl_target.h"

typedef struct wl_hw_context_t {
  hw_context_t base;
} wl_hw_context_t;

wl_hw_context_t *
wl_hw_context_create(
  wl_target_t *target,
  int32_t width,
  int32_t height)
{
  return NULL;
}

void
wl_hw_context_destroy(
  wl_hw_context_t *context)
{
}

bool
wl_hw_context_resize(
  wl_hw_context_t *context,
  int32_t width,
  int32_t height)
{
  return false;
}

void
wl_hw_context_present(
  wl_hw_context_t *context)
{
}

#else

const int wl_hw_context = 0;

#endif /* HAS_WAYLAND */
