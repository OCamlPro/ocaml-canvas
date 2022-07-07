/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <memory.h>
#include <math.h>
#include <assert.h>

#include "config.h"
#include "tuples.h"
#include "target.h"
#include "present_data.h"
#include "color.h"
#include "image_data.h"
#include "impexp.h"
#include "surface.h"
#include "surface_internal.h"

#ifdef HAS_GDI
#include "gdi/gdi_surface.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_surface.h"
#endif
#ifdef HAS_X11
#include "x11/x11_surface.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_surface.h"
#endif

static surface_t *
_surface_create_internal(
  int32_t width,
  int32_t height,
  color_t_ *data) // May be NULL
{
  assert(width > 0);
  assert(height > 0);

  surface_t *s = (surface_t *)calloc(1, sizeof(surface_t));
  if (s == NULL) {
    return NULL;
  }

  s->impl = NULL;
  s->data = data;
  s->width = width;
  s->height = height;

  return s;
}

surface_t *
surface_create(
  int32_t width,
  int32_t height)
{
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
  if (data == NULL) {
    return NULL;
  }

  surface_t *s = _surface_create_internal(width, height, data);
  if (s == NULL) {
    free(data);
    return NULL;
  }

  return s;
}

surface_t *
surface_create_from_image_data(
  image_data_t *image_data)
{
  assert(image_data != NULL);
  assert(image_data_valid(image_data) == true);

  return _surface_create_internal(image_data->width,
                                  image_data->height,
                                  image_data->data);
}

surface_t *
surface_create_from_png(
  const char *filename)
{
  assert(filename != NULL);

  int32_t width = 0;
  int32_t height = 0;
  color_t *data = NULL;

  bool res = impexp_import_png(&data, &width, &height, 0, 0, filename);
  if (res == false) {
    return NULL;
  }

  surface_t *s = _surface_create_internal(width, height, data);
  if (s == NULL) {
    free(data);
    return NULL;
  }

  return s;
}

surface_t *
surface_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(width > 0);
  assert(height > 0);

  surface_t *s = _surface_create_internal(width, height, NULL);
  if (s == NULL) {
    return NULL;
  }

  switch_IMPL() {
    case_GDI(s->impl = (surface_impl_t *)
             surface_create_gdi_impl((gdi_target_t *)target,
                                     width, height, &s->data));
    case_QUARTZ(s->impl = (surface_impl_t *)
                surface_create_qtz_impl((qtz_target_t *)target,
                                        width, height, &s->data));
    case_X11(s->impl = (surface_impl_t *)
             surface_create_x11_impl((x11_target_t *)target,
                                     width, height, &s->data));
    case_WAYLAND(s->impl = (surface_impl_t *)
                 surface_create_wl_impl((wl_target_t *)target,
                                        width, height, &s->data));
    default_fail();
  }
  if (s->impl == NULL) {
    free(s);
    return NULL;
  }
  assert(s->data != NULL);

  return s;
}

void
surface_destroy(
  surface_t *s)
{
  assert(s != NULL);

  if (s->impl != NULL) {
    switch_IMPL() {
      case_GDI(surface_destroy_gdi_impl((surface_impl_gdi_t *)s->impl);
               s->data = NULL; /* freed but not nulled */);
      case_QUARTZ(surface_destroy_qtz_impl((surface_impl_qtz_t *)s->impl);
                  /* s->data = NULL; // if not done already -- check */);
      case_X11(surface_destroy_x11_impl((surface_impl_x11_t *)s->impl));
      case_WAYLAND(surface_destroy_wl_impl((surface_impl_wl_t *)s->impl));
      default_fail();
    }
    free(s->impl);
  }

  if (s->data) {
    free(s->data);
  }

  free(s);
}

pair_t(int32_t)
surface_get_size(
  const surface_t *s)
{
  assert(s != NULL);

  return pair(int32_t, s->width, s->height);
}

static void
_surface_copy_to_buffer(
  surface_t *s,
  color_t_ *data,
  int32_t width,
  int32_t height)
{
  assert(s != NULL);
  assert(s->data != NULL);
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);

  uint32_t min_width = width < s->width ? width : s->width;
  uint32_t min_height = height < s->height ? height : s->height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      data[i * width + j] = s->data[i * s->width + j];
    }
  }
}

bool
surface_resize(
  surface_t *s,
  int32_t width,
  int32_t height)
{
  assert(s != NULL);
  assert(s->data != NULL);

  if ((width <= 0) || (height <= 0)) {
    return false;
  }

  color_t_ *data = NULL;

// TODO: fill extra data with background color

  if (s->impl == NULL) {

    data = (color_t_ *)calloc(width * height, sizeof(color_t_));
    if (data == NULL) {
      return false;
    }

    _surface_copy_to_buffer(s, data, width, height);

    free(s->data);

  } else {

    bool result = false;
    switch_IMPL() {
      case_GDI(result =
               surface_resize_gdi_impl((surface_impl_gdi_t *)s->impl,
                                       s->width, s->height, &s->data,
                                       width, height, &data));
      case_QUARTZ(result =
                  surface_resize_qtz_impl((surface_impl_qtz_t *)s->impl,
                                          s->width, s->height, &s->data,
                                          width, height, &data));
      case_X11(result =
               surface_resize_x11_impl((surface_impl_x11_t *)s->impl,
                                       s->width, s->height, &s->data,
                                       width, height, &data));
      case_WAYLAND(result =
                   surface_resize_wl_impl((surface_impl_wl_t *)s->impl,
                                          s->width, s->height, &s->data,
                                          width, height, &data));
      default_fail();
    }
    if (result == false) {
      return false;
    }

  }

  s->data = data;
  s->width = width;
  s->height = height;

  return true;
}

void
surface_present(
  surface_t *s,
  present_data_t *present_data)
{
  assert(s != NULL);
  assert(s->impl != NULL);
  assert(present_data != NULL);

  switch_IMPL() {
    case_GDI(surface_present_gdi_impl((surface_impl_gdi_t *)s->impl,
                                      s->width, s->height,
                                      &present_data->gdi));
    case_QUARTZ(surface_present_qtz_impl((surface_impl_qtz_t *)s->impl,
                                         s->width, s->height,
                                         &present_data->qtz));
    case_X11(surface_present_x11_impl((surface_impl_x11_t *)s->impl,
                                      s->width, s->height,
                                      &present_data->x11));
    case_WAYLAND(surface_present_wl_impl((surface_impl_wl_t *)s->impl,
                                         s->width, s->height,
                                         &present_data->wl));
    default_fail();
  }
}

void
surface_blit(
  surface_t *ds,
  int32_t dx,
  int32_t dy,
  const surface_t *ss,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(ds != NULL);
  assert(ds->data != NULL);
  assert(ss != NULL);
  assert(ss->data != NULL);

  if (dx < 0) {
    sx -= dx;
    width += dx;
    dx = 0;
  }
  if (dy < 0) {
    sy -= dy;
    height += dy;
    dy = 0;
  }

  if (sx < 0) {
    dx -= sx;
    width += sx;
    sx = 0;
  }
  if (sy < 0) {
    dy -= sy;
    height += sy;
    sy = 0;
  }

  if (dx + width > ds->width) {
    width = ds->width -dx;
  }
  if (dy + height > ds->height) {
    height = ds->height -dy;
  }

  if (sx + width > ss->width) {
    width = ss->width -sx;
  }
  if (sy + height > ss->height) {
    height = ss->height -sy;
  }

  for (int32_t i = 0; i < height; ++i) {
    for (int32_t j = 0; j < width; ++j) {
      ds->data[(dy+i) * ds->width + (dx+j)] =
        ss->data[(sy+i) * ss->width + (sx+j)];
    }
  }

}

color_t_
surface_get_pixel(
  const surface_t *s,
  int32_t x,
  int32_t y)
{
  assert(s != NULL);
  assert(s->data != NULL);

  color_t_ color = color_black;

  if ((x >= 0) && (x < s->width) && (y >= 0) && (y < s->height)) {
    color = s->data[y * s->width + x];
  }

  return color;
}

void
surface_set_pixel(
  surface_t *s,
  int32_t x,
  int32_t y,
  color_t_ color)
{
  assert(s != NULL);
  assert(s->data != NULL);

  if ((x >= 0) && (x < s->width) && (y >= 0) && (y < s->height)) {
    s->data[y * s->width + x] = color;
  }
}

image_data_t
surface_get_image_data(
  const surface_t *ss,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(ss != NULL);
  assert(width > 0);
  assert(height > 0);

  image_data_t dd = image_data(width, height, NULL);
  if (image_data_valid(dd) == false) {
    return dd;
  }

  int32_t dx = 0;
  int32_t dy = 0;

  if (sx < 0) { dx -= sx; width += sx; sx = 0; }
  if (sy < 0) { dy -= sy; height += sy; sy = 0; }

  if (sx + width > ss->width) { width = ss->width - sx; }
  if (sy + height > ss->height) { height = ss->height - sy; }

  if ((width > 0) && (height > 0)) {
    for (int32_t i = 0; i < height; ++i) {
      memcpy(&dd.data[(dy+i) * dd.width + dx],
             &ss->data[(sy+i) * ss->width + sx],
             width * COLOR_SIZE);
    }
  }

  return dd;
}

void
surface_set_image_data(
  surface_t *ds,
  int32_t dx,
  int32_t dy,
  const image_data_t *sd,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height)
{
  assert(ds != NULL);
  assert(sd != NULL);
  assert(width > 0);
  assert(height > 0);

  if (dx < 0) { sx -= dx; width += dx; dx = 0; }
  if (dy < 0) { sy -= dy; height += dy; dy = 0; }

  if (sx < 0) { dx -= sx; width += sx; sx = 0; }
  if (sy < 0) { dy -= sy; height += sy; sy = 0; }

  if (dx + width > ds->width) { width = ds->width - dx; }
  if (dy + height > ds->height) { height = ds->height - dy; }

  if (sx + width > sd->width) { width = sd->width - sx; }
  if (sy + height > sd->height) { height = sd->height - sy; }

  if ((width > 0) && (height > 0)) {
    for (int32_t i = 0; i < height; ++i) {
      memcpy(&ds->data[(dy+i) * ds->width + dx],
             &sd->data[(sy+i) * sd->width + sx],
             width * COLOR_SIZE);
    }
  }
}

image_data_t
surface_get_raw_image_data(
  surface_t *s)
{
  assert(s != NULL);
  assert(s->data != NULL);

  return image_data(s->width, s->height, s->data);
}

bool
surface_export_png(
  const surface_t *s,
  const char *filename)
{
  assert(s != NULL);
  assert(filename != NULL);

  return impexp_export_png(s->data, s->width, s->height, filename);
}

bool
surface_import_png(
  surface_t *s,
  int32_t x,
  int32_t y,
  const char *filename)
{
  assert(s != NULL);
  assert(filename != NULL);

  return impexp_import_png(&s->data, &s->width, &s->height, x, y, filename);
}
