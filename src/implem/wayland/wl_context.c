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
#include "../color.h"
#include "wl_target.h"
#include "wl_present_data.h"

typedef struct context_impl_wl_t {
  impl_type_t type;
} context_impl_wl_t;

context_impl_wl_t *
context_create_wl_impl(
  wl_target_t *wl_target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  return NULL;
}

void
context_destroy_wl_impl(
  context_impl_wl_t *impl)
{
}

bool
context_resize_wl_impl(
  context_impl_wl_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  return false;
}

void
context_present_wl_impl(
  context_impl_wl_t *impl,
  int32_t width,
  int32_t height,
  wl_present_data_t *present_data)
{
}

#else

const int wl_context = 0;

#endif /* HAS_WAYLAND */
