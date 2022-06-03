/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_X11

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>

#include "../config.h"
#include "../color.h"
#include "x11_backend_internal.h"
#include "x11_window.h"
#include "x11_target.h"

typedef struct surface_impl_x11_t {
  impl_type_t type;
  xcb_image_t *img;
  xcb_window_t wid;
  xcb_gcontext_t cid;
} surface_impl_x11_t;

static xcb_image_t *
_surface_create_x11_image(
  xcb_connection_t *c,
  uint8_t depth,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(c != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  *data = (color_t_ *)calloc(width * height, sizeof(color_t_));
  if (*data == NULL) {
    return NULL;
  }

  xcb_image_t *img =
    xcb_image_create_native(c, width, height,
                            XCB_IMAGE_FORMAT_Z_PIXMAP, depth,
                            NULL, 0, (uint8_t *)*data);
  if (img == NULL) {
    free(*data);
    *data = NULL;
    return NULL;
  }

  return img;
}

surface_impl_x11_t *
surface_create_x11_impl(
  x11_target_t *target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(target != NULL);
  assert(target->wid != XCB_WINDOW_NONE);
  // cid ?
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  surface_impl_x11_t *impl =
    (surface_impl_x11_t *)calloc(1, sizeof(surface_impl_x11_t));
  if (impl == NULL) {
    return NULL;
  }

// allow xcb / SHM

  xcb_image_t *img = _surface_create_x11_image(x11_back->c,
                                               x11_back->screen->root_depth,
                                               width, height, data);
  if (img == NULL) {
    free(impl);
    return NULL;
  }

  impl->type = IMPL_X11;
  impl->wid = target->wid;
  impl->cid = target->cid;
  impl->img = img;

  return impl;
}

void
surface_destroy_x11_impl(
  surface_impl_x11_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_X11);

  if (impl->img) {
    xcb_image_destroy(impl->img);
  }
}

static void
_raw_surface_copy(
  color_t_ *s_data,
  int32_t s_width,
  int32_t s_height,
  color_t_ *d_data,
  int32_t d_width,
  int32_t d_height)
{
  assert(s_data != NULL);
  assert(s_width > 0);
  assert(s_height > 0);
  assert(d_data != NULL);
  assert(d_width > 0);
  assert(d_height > 0);
  uint32_t min_width = d_width < s_width ? d_width : s_width;
  uint32_t min_height = d_height < s_height ? d_height : s_height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      d_data[i * d_width + j] = s_data[i * s_width + j];
    }
  }
}

bool
surface_resize_x11_impl(
  surface_impl_x11_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  assert(impl != NULL);
  assert(s_width > 0);
  assert(s_height  > 0);
  assert(s_data != NULL);
  assert(*s_data != NULL);
  assert(d_width > 0);
  assert(d_height  > 0);
  assert(d_data != NULL);
  assert(*d_data == NULL);

  xcb_image_t *img = _surface_create_x11_image(x11_back->c,
                                               x11_back->screen->root_depth,
                                               d_width, d_height, d_data);
  if (img == NULL) {
    return false;
  }

  _raw_surface_copy(*s_data, s_width, s_height, *d_data, d_width, d_height);

  if (impl->img) {
    xcb_image_destroy(impl->img);
  }
  free(*s_data);

  impl->img = img;

  return true;
}

void
surface_present_x11_impl(
  surface_impl_x11_t *impl,
  int32_t width,
  int32_t height,
  x11_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height  > 0);

  xcb_image_put(x11_back->c, impl->wid, impl->cid, impl->img, 0, 0, 0);
  xcb_flush(x11_back->c);
}

#else

const int x11_surface = 0;

#endif /* HAS_X11 */
