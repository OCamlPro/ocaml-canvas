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
#include <math.h>
#include <assert.h>

#include "config.h"
#include "target.h"
#include "present_data.h"
#include "color.h"
#include "pixmap.h"
#include "context.h"
#include "context_internal.h"

#ifdef HAS_GDI
#include "gdi/gdi_context.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_context.h"
#endif
#ifdef HAS_X11
#include "x11/x11_context.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_context.h"
#endif

static context_t *
_context_create_internal(
  int32_t width,
  int32_t height,
  color_t_ *data) // May be NULL
{
  assert(width > 0);
  assert(height > 0);

  context_t *c = (context_t *)calloc(1, sizeof(context_t));
  if (c == NULL) {
    return NULL;
  }

  c->impl = NULL;
  c->data = data;
  c->width = width;
  c->height = height;

  return c;
}

context_t *
context_create(
  int32_t width,
  int32_t height)
{
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
  if (data == NULL) {
    return NULL;
  }

  context_t *c = _context_create_internal(width, height, data);
  if (c == NULL) {
    free(data);
    return NULL;
  }

  return c;
}

context_t *
context_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  context_t *c =
    _context_create_internal(pixmap->width, pixmap->height, pixmap->data);
  if (c != NULL) {
    pixmap->data = NULL;
    pixmap->width = 0;
    pixmap->height = 0;
  }

  return c;
}

context_t *
context_create_onscreen(
  target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(width > 0);
  assert(height > 0);

  context_t *c = _context_create_internal(width, height, NULL);
  if (c == NULL) {
    return NULL;
  }

  switch_IMPL() {
    case_GDI(c->impl = (context_impl_t *)
             context_create_gdi_impl((gdi_target_t *)target,
                                     width, height, &c->data));
    case_QUARTZ(c->impl = (context_impl_t *)
                context_create_qtz_impl((qtz_target_t *)target,
                                        width, height, &c->data));
    case_X11(c->impl = (context_impl_t *)
             context_create_x11_impl((x11_target_t *)target,
                                     width, height, &c->data));
    case_WAYLAND(c->impl = (context_impl_t *)
                 context_create_wl_impl((wl_target_t *)target,
                                        width, height, &c->data));
    default_fail();
  }
  if (c->impl == NULL) {
    free(c);
    return NULL;
  }
  assert(c->data != NULL);

  return c;
}

void
context_destroy(
  context_t *c)
{
  assert(c != NULL);

  if (c->impl != NULL) {
    switch_IMPL() {
      case_GDI(context_destroy_gdi_impl((context_impl_gdi_t *)c->impl);
               c->data = NULL; /* freed but not nulled */);
      case_QUARTZ(context_destroy_qtz_impl((context_impl_qtz_t *)c->impl));
      case_X11(context_destroy_x11_impl((context_impl_x11_t *)c->impl));
      case_WAYLAND(context_destroy_wl_impl((context_impl_wl_t *)c->impl));
      default_fail();
    }
    free(c->impl); // let the impl do that ?
  }

  if (c->data) {
    free(c->data);
  }

  free(c);
}

static void
_context_copy_to_buffer(
  context_t *c,
  color_t_ *data,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);

  uint32_t min_width = width < c->width ? width : c->width;
  uint32_t min_height = height < c->height ? height : c->height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      data[i * width + j] = c->data[i * c->width + j];
    }
  }
}

bool
context_resize(
  context_t *c,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->data != NULL);

  if ((width <= 0) || (height <= 0)) {
    return false;
  }

  color_t_ *data = NULL;

  // TODO: fill extra data with background color

  if (c->impl == NULL) {

    data = (color_t_ *)calloc(width * height, sizeof(color_t_));
    if (data == NULL) {
      return false;
    }

    _context_copy_to_buffer(c, data, width, height);

    free(c->data);

  } else {

    bool result = false;
    switch_IMPL() {
      case_GDI(result =
               context_resize_gdi_impl((context_impl_gdi_t *)c->impl,
                                       c->width, c->height, &c->data,
                                       width, height, &data));
      case_QUARTZ(result =
                  context_resize_qtz_impl((context_impl_qtz_t *)c->impl,
                                          c->width, c->height, &c->data,
                                          width, height, &data));
      case_X11(result =
               context_resize_x11_impl((context_impl_x11_t *)c->impl,
                                       c->width, c->height, &c->data,
                                       width, height, &data));
      case_WAYLAND(result =
                   context_resize_wl_impl((context_impl_wl_t *)c->impl,
                                          c->width, c->height, &c->data,
                                          width, height, &data));
      default_fail();
    }
    if (result == false) {
      return false;
    }

  }

  c->data = data;
  c->width = width;
  c->height = height;

  return true;
}

void
context_present(
  context_t *c,
  present_data_t *present_data)
{
  assert(c != NULL);
  assert(c->impl != NULL);
  assert(present_data != NULL);

  switch_IMPL() {
    case_GDI(context_present_gdi_impl((context_impl_gdi_t *)c->impl,
                                      c->width, c->height,
                                      &present_data->gdi));
    case_QUARTZ(context_present_qtz_impl((context_impl_qtz_t *)c->impl,
                                         c->width, c->height,
                                         &present_data->qtz));
    case_X11(context_present_x11_impl((context_impl_x11_t *)c->impl,
                                      c->width, c->height,
                                      &present_data->x11));
    case_WAYLAND(context_present_wl_impl((context_impl_wl_t *)c->impl,
                                         c->width, c->height,
                                         &present_data->wl));
    default_fail();
  }
}

pixmap_t
context_get_raw_pixmap(
  context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);

  return pixmap(c->width, c->height, c->data);
}
