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
#include <assert.h>

#include "config.h"
#include "target.h"
#include "pixmap.h"
#include "color.h"
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

  context_t *c = (context_t *)calloc(1, sizeof(context_t));
  if (c == NULL) {
    free(data);
    return NULL;
  }

  c->offscreen = true;
  c->data = data;
  c->width = width;
  c->height = height;

  return c;
}

context_t *
context_create_from_pixmap(
  pixmap_t *pixmap)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap) == true);

  context_t *c = (context_t *)calloc(1, sizeof(context_t));
  if (c == NULL) {
    return NULL;
  }

  c->offscreen = true;
  c->data = pixmap->data;
  c->width = pixmap->width;
  c->height = pixmap->height;

  pixmap->data = NULL;
  pixmap->width = 0;
  pixmap->height = 0;

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

  context_t *c = NULL;
  switch_IMPL() {
    case_GDI(c = (context_t *)gdi_context_create((gdi_target_t *)target,
                                                 width, height));
    case_QUARTZ(c = (context_t *)qtz_context_create((qtz_target_t *)target,
                                                    width, height));
    case_X11(c = (context_t *)x11_context_create((x11_target_t *)target,
                                                 width, height));
    case_WAYLAND(c = (context_t *)wl_context_create((wl_target_t *)target,
                                                    width, height));
    default_fail();
  }
  if (c == NULL) {
    return NULL;
  }
  c->offscreen = false;

  return c;
}

void
context_destroy(
  context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);

  if (c->offscreen == true) {
    free(c->data);
    free(c);
  } else {
    switch_IMPL() {
      case_GDI(gdi_context_destroy((gdi_context_t *)c));
      case_QUARTZ(qtz_context_destroy((qtz_context_t *)c));
      case_X11(x11_context_destroy((x11_context_t *)c));
      case_WAYLAND(wl_context_destroy((wl_context_t *)c));
      default_fail();
    }
  }
}

void
_context_copy_to_buffer(
  context_t *c,
  color_t_ *data,
  int32_t width,
  int32_t height)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);
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
  assert(c->width > 0);
  assert(c->height > 0);

  if ((width <= 0) || (height <= 0)) {
    return false;
  }

  if ((width == c->width) && (height == c->height)) {
    return true;
  }

  // TODO: fill extra data with background color

  if (c->offscreen == true) {

    color_t_ *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
    if (data == NULL) {
      return false;
    }

    _context_copy_to_buffer(c, data, width, height);

    free(c->data);

    c->data = data;
    c->width = width;
    c->height = height;

    return true;

  } else {

    bool result = false;

    switch_IMPL() {
      case_GDI(result =
               gdi_context_resize((gdi_context_t *)c, width, height));
      case_QUARTZ(result =
                  qtz_context_resize((qtz_context_t *)c, width, height));
      case_X11(result =
               x11_context_resize((x11_context_t *)c, width, height));
      case_WAYLAND(result =
                   wl_context_resize((wl_context_t *)c, width, height));
      default_fail();
    }

    return result;
  }
}

void
context_present(
  context_t *c)
{
  assert(c != NULL);
  assert(c->offscreen == false);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  switch_IMPL() {
    case_GDI(gdi_context_present((gdi_context_t *)c));
    case_QUARTZ(qtz_context_present((qtz_context_t *)c));
    case_X11(x11_context_present((x11_context_t *)c));
    case_WAYLAND(wl_context_present((wl_context_t *)c));
    default_fail();
  }
}

pixmap_t
context_get_raw_pixmap(
  context_t *c)
{
  assert(c != NULL);
  assert(c->data != NULL);
  assert(c->width > 0);
  assert(c->height > 0);

  return pixmap(c->width, c->height, c->data);
}
