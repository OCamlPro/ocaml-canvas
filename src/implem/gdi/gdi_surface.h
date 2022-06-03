/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_SURFACE_H
#define __GDI_SURFACE_H

#include <stdint.h>
#include <stdbool.h>

#include <windows.h>

#include "../color.h"
#include "gdi_target.h"
#include "gdi_present_data.h"

typedef struct surface_impl_gdi_t surface_impl_gdi_t;

surface_impl_gdi_t *
surface_create_gdi_impl(
  gdi_target_t *target,
  int32_t width,
  int32_t height,
  color_t_ **data);

void
surface_destroy_gdi_impl(
  surface_impl_gdi_t *impl);

bool
surface_resize_gdi_impl(
  surface_impl_gdi_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data);

void
surface_present_gdi_impl(
  surface_impl_gdi_t *impl,
  int32_t width,
  int32_t height,
  gdi_present_data_t *present_data);

#endif /* __GDI_SURFACE_H */
