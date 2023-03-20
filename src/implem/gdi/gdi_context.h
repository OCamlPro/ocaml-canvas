/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_CONTEXT_H
#define __GDI_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "../color.h"
#include "gdi_target.h"

typedef struct gdi_context_t gdi_context_t;

gdi_context_t *
gdi_context_create(
  gdi_target_t *target,
  int32_t width,
  int32_t height);

void
gdi_context_destroy(
  gdi_context_t *context);

bool
gdi_context_resize(
  gdi_context_t *context,
  int32_t width,
  int32_t height);

void
gdi_context_present(
  gdi_context_t *context);

#endif /* __GDI_CONTEXT_H */
