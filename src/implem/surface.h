/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __SURFACE_H
#define __SURFACE_H

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "target.h"
#include "present_data.h"
#include "pixmap.h"

typedef struct surface_t surface_t;

surface_t *
surface_create(
  int32_t width,
  int32_t height);

// Creates a surface from a pixmap
// The data pointer is transfered to the surface
// (thus removed from the pixmap); if you kept
// a copy, do NOT free it !
surface_t *
surface_create_from_pixmap(
  pixmap_t *pixmap);

surface_t *
surface_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height);

void
surface_destroy(
  surface_t *s);

bool
surface_resize(
  surface_t *s,
  int32_t width,
  int32_t height);

void
surface_present(
  surface_t *s,
  present_data_t *present_data);

// Direct access to the surface pixels
// Do NOT free the data pointer !
pixmap_t
surface_get_raw_pixmap(
  surface_t *s);

#endif /* __SURFACE_H */
