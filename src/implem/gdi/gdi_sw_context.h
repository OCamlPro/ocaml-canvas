/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_SW_CONTEXT_H
#define __GDI_SW_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "gdi_target.h"

typedef struct gdi_sw_context_t gdi_sw_context_t;

gdi_sw_context_t *
gdi_sw_context_create(
  gdi_target_t *target,
  int32_t width,
  int32_t height);

void
gdi_sw_context_destroy(
  gdi_sw_context_t *context);

bool
gdi_sw_context_resize(
  gdi_sw_context_t *context,
  int32_t width,
  int32_t height);

void
gdi_sw_context_present(
  gdi_sw_context_t *context);

#endif /* __GDI_SW_CONTEXT_H */
