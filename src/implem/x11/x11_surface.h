/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_SURFACE_H
#define __X11_SURFACE_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xcb.h>

#include "../color.h"
#include "x11_target.h"
#include "x11_present_data.h"

typedef struct surface_impl_x11_t surface_impl_x11_t;

surface_impl_x11_t *
surface_create_x11_impl(
  x11_target_t *target,
  int32_t width,
  int32_t height,
  color_t_ **data);

void
surface_destroy_x11_impl(
  surface_impl_x11_t *impl);

bool
surface_resize_x11_impl(
  surface_impl_x11_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data);

void
surface_present_x11_impl(
  surface_impl_x11_t *impl,
  int32_t width,
  int32_t height,
  x11_present_data_t *present_data);

#endif /* __X11_SURFACE_H */
