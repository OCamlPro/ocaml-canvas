/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __CONTEXT_H
#define __CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "target.h"
#include "pixmap.h"

typedef struct context_t context_t;

context_t *
context_create(
  int32_t width,
  int32_t height);

// Creates a context from a pixmap
// The data pointer is transfered to the context
// (thus removed from the pixmap); if you kept
// a copy, do NOT free it !
context_t *
context_create_from_pixmap(
  pixmap_t *pixmap);

context_t *
context_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height);

void
context_destroy(
  context_t *c);

bool
context_resize(
  context_t *c,
  int32_t width,
  int32_t height);

void
context_present(
  context_t *c);

// Direct access to the context pixels
// Do NOT free the data pointer !
pixmap_t
context_get_raw_pixmap(
  context_t *c);

#endif /* __CONTEXT_H */
