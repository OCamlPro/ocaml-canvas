/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __X11_HW_CONTEXT_H
#define __X11_HW_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "x11_target.h"

typedef struct x11_hw_context_t x11_hw_context_t;

x11_hw_context_t *
x11_hw_context_create(
  x11_target_t *target,
  int32_t width,
  int32_t height);

void
x11_hw_context_destroy(
  x11_hw_context_t *context);

bool
x11_hw_context_resize(
  x11_hw_context_t *context,
  int32_t width,
  int32_t height);

void
x11_hw_context_present(
  x11_hw_context_t *context);

#endif /* __X11_HW_CONTEXT_H */
