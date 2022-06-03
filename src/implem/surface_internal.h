/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __SURFACE_INTERNAL_H
#define __SURFACE_INTERNAL_H

#include <stdint.h>

#include "color.h"

typedef struct surface_impl_t surface_impl_t;

typedef struct surface_t {
  surface_impl_t *impl;
  color_t_ *data;
  int32_t width;
  int32_t height;
} surface_t;

#endif /* __SURFACE_INTERNAL_H */
