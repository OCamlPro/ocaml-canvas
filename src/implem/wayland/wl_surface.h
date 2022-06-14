/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_SURFACE_H
#define __WL_SURFACE_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>


#include "../color.h"
#include "wl_target.h"
#include "wl_present_data.h"

typedef struct surface_impl_wl_t surface_impl_wl_t;

surface_impl_wl_t *
surface_create_wl_impl(
  wl_target_t *wl_target,
  int32_t width,
  int32_t height,
  color_t_ **data);

void
surface_destroy_wl_impl(
  surface_impl_wl_t *impl);

bool
surface_resize_wl_impl(
  surface_impl_wl_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data);

void
surface_present_wl_impl(
  surface_impl_wl_t *impl,
  int32_t width,
  int32_t height,
  wl_present_data_t *present_data);

#endif /* __WL_SURFACE_H */
