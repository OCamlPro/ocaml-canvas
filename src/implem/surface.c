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
#include "target.h"
#include "present_data.h"
#include "color.h"
#include "pixmap.h"
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
surface_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  surface_t *s =
    _surface_create_internal(pixmap->width, pixmap->height, pixmap->data);
  if (s != NULL) {
    pixmap->data = NULL;
    pixmap->width = 0;
    pixmap->height = 0;
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

pixmap_t
surface_get_raw_pixmap(
  surface_t *s)
{
  assert(s != NULL);
  assert(s->data != NULL);

  return pixmap(s->width, s->height, s->data);
}
