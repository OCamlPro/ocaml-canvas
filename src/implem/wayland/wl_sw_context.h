/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_SW_CONTEXT_H
#define __WL_SW_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "wl_target.h"

typedef struct wl_sw_context_t wl_sw_context_t;

wl_sw_context_t *
wl_sw_context_create(
  wl_target_t *target,
  int32_t width,
  int32_t height);

void
wl_sw_context_destroy(
  wl_sw_context_t *context);

bool
wl_sw_context_resize(
  wl_sw_context_t *context,
  int32_t width,
  int32_t height);

void
wl_sw_context_present(
  wl_sw_context_t *context);

#endif /* __WL_SW_CONTEXT_H */
