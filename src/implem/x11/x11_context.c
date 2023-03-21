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
#include "../context_internal.h"
#include "x11_backend_internal.h"
#include "x11_target.h"

typedef struct x11_context_t {
  context_t base;
  xcb_image_t *img;
  xcb_window_t wid;
  xcb_gcontext_t cid;
} x11_context_t;

static xcb_image_t *
_x11_context_create_image(
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

x11_context_t *
x11_context_create(
  x11_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->wid != XCB_WINDOW_NONE);
  assert(width > 0);
  assert(height > 0);

  x11_context_t *context =
    (x11_context_t *)calloc(1, sizeof(x11_context_t));
  if (context == NULL) {
    return NULL;
  }

  // TODO: allow xcb / SHM
  color_t_ *data = NULL;
  xcb_image_t *img =
    _x11_context_create_image(x11_back->c, x11_back->screen->root_depth,
                              width, height, &data);
  if (img == NULL) {
    assert(data == NULL);
    free(context);
    return NULL;
  }
  assert(data != NULL);

  xcb_gcontext_t cid = xcb_generate_id(x11_back->c);
  xcb_create_gc(x11_back->c, cid, target->wid,
                XCB_GC_GRAPHICS_EXPOSURES, (uint32_t[]){ 1 });

  context->base.data = data;
  context->base.width = width;
  context->base.height = height;
  context->img = img;
  context->wid = target->wid;
  context->cid = cid;

  return context;
}

void
x11_context_destroy(
  x11_context_t *context)
{
  assert(context != NULL);

  if (context->cid != XCB_NONE) {
    xcb_free_gc(x11_back->c, context->cid);
  }

  if (context->img != NULL) {
    xcb_image_destroy(context->img);
    free(context->base.data); /* We allocated it, we have to free it */
  }

  free(context);
}

bool
x11_context_resize(
  x11_context_t *context,
  int32_t width,
  int32_t height)
{
  assert(context != NULL);
  assert(context->base.data != NULL);
  assert(context->base.width > 0);
  assert(context->base.height > 0);
  assert(context->img != NULL);
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = NULL;
  xcb_image_t *img =
    _x11_context_create_image(x11_back->c, x11_back->screen->root_depth,
                              width, height, &data);
  if (img == NULL) {
    assert(data == NULL);
    return false;
  }
  assert(data != NULL);

  _context_copy_to_buffer(&context->base, data, width, height);

  xcb_image_destroy(context->img);

  context->base.data = data;
  context->base.width = width;
  context->base.height = height;
  context->img = img;

  return true;
}

void
x11_context_present(
  x11_context_t *context)
{
  assert(context != NULL);
  assert(context->base.width > 0);
  assert(context->base.height > 0);
  assert(context->img != NULL);
  assert(context->wid != XCB_WINDOW_NONE);
  assert(context->cid != XCB_NONE);

  xcb_image_put(x11_back->c, context->wid, context->cid,
                context->img, 0, 0, 0);
  xcb_flush(x11_back->c);
}

#else

const int x11_context = 0;

#endif /* HAS_X11 */
