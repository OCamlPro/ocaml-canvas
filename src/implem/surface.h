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
#include "tuples.h"
#include "target.h"
#include "present_data.h"
#include "color.h"
#include "image_data.h"

typedef struct surface_t surface_t;

surface_t *
surface_create(
  int32_t width,
  int32_t height);

// Creates a surface from image data
// The data pointer is transfered to the surface; do NOT free it
surface_t *
surface_create_from_image_data(
  image_data_t *image_data);

surface_t *
surface_create_from_png(
  const char *filename);

surface_t *
surface_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height);

void
surface_destroy(
  surface_t *s);

pair_t(int32_t)
surface_get_size(
  const surface_t *s);

bool
surface_resize(
  surface_t *s,
  int32_t width,
  int32_t height);

void
surface_present(
  surface_t *s,
  present_data_t *present_data);

void
surface_blit(
  surface_t *ds,
  int32_t dx,
  int32_t dy,
  const surface_t *ss,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

color_t_
surface_get_pixel(
  const surface_t *s,
  int32_t x,
  int32_t y);

void
surface_set_pixel(
  surface_t *s,
  int32_t x,
  int32_t y,
  color_t_ color);

// Creates a copy of the surface pixels
// Be sure to free the data pointer when done
image_data_t
surface_get_image_data(
  const surface_t *ss,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

void
surface_set_image_data(
  surface_t *ds,
  int32_t dx,
  int32_t dy,
  const image_data_t *sd,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

// Direct access to the surface pixels
// Do NOT free the data pointer !
image_data_t
surface_get_raw_image_data(
  surface_t *s);

bool
surface_export_png(
  const surface_t *s,
  const char *filename);

bool
surface_import_png(
  surface_t *s,
  int32_t x,
  int32_t y,
  const char *filename);

#endif /* __SURFACE_H */
