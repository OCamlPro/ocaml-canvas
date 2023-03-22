/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __SW_CONTEXT_INTERNAL_H
#define __SW_CONTEXT_INTERNAL_H

#include <stdint.h>

#include "color.h"
#include "pixmap.h"
#include "context_internal.h"

typedef struct sw_context_t {
  context_t base;
  color_t_ *data;
  pixmap_t clip_region; // temporary
} sw_context_t;

void
_sw_context_copy_to_buffer(
  sw_context_t *c,
  color_t_ *data,
  int32_t width,
  int32_t height);

#endif /* __SW_CONTEXT_INTERNAL_H */
