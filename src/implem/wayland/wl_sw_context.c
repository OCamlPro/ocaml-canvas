/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <sys/mman.h>

#include <wayland-client.h>

#include "../config.h"
#include "../color.h"
#include "../context_internal.h"
#include "../sw_context_internal.h"
#include "wl_target.h"
#include "wl_memory.h"

typedef struct wl_sw_context_t {
  sw_context_t base;
  struct wl_buffer *wl_buffer;
  struct wl_surface *wl_surface;
  uint8_t *data;
  uint32_t size;
} wl_sw_context_t;

static void
_wl_sw_buffer_release(
  void *data,
  struct wl_buffer *wl_buffer)
{
  // handle buffer release event (maybe useful later)
  // struct wl_window_t *wl_window = data;
}

static const struct wl_buffer_listener
_wl_sw_buffer_listener = {
  .release = _wl_sw_buffer_release,
};

wl_sw_context_t *
wl_sw_context_create(
  wl_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->wl_surface != NULL);
  assert(width > 0);
  assert(height > 0);

  wl_sw_context_t *context =
    (wl_sw_context_t *)calloc(1, sizeof(wl_sw_context_t));
  if (context == NULL) {
    return NULL;
  }

  uint8_t *data = NULL;
  struct wl_buffer *wl_buffer = wl_create_buffer(width, height, &data);
  if (wl_buffer == NULL) {
    assert(data == NULL);
    free(context);
    return false;
  }
  assert(data != NULL);

  wl_buffer_add_listener(wl_buffer, &_wl_sw_buffer_listener,
                         wl_surface_get_user_data(target->wl_surface));

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = (color_t_ *)data;
  context->wl_buffer = wl_buffer;
  context->wl_surface = target->wl_surface;
  context->data = data;
  context->size = width * height * COLOR_SIZE;

  return context;
}

void
wl_sw_context_destroy(
  wl_sw_context_t *context)
{
  assert(context != NULL);

  if (context->wl_buffer != NULL) {
    wl_buffer_destroy(context->wl_buffer);
    munmap(context->data, context->size);
  }

  free(context);
}

bool
wl_sw_context_resize(
  wl_sw_context_t *context,
  int32_t width,
  int32_t height)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->base.data != NULL);
  assert(context->wl_buffer != NULL);
  assert(context->data == (uint8_t *)context->base.data);
  assert(width > 0);
  assert(height > 0);

  wl_surface_attach(context->wl_surface, NULL, 0, 0);

  uint8_t *data = NULL;
  struct wl_buffer *wl_buffer = wl_create_buffer(width, height, &data);
  if (wl_buffer == NULL) {
    assert(data == NULL);
    return false;
  }
  assert(data != NULL);

  wl_buffer_add_listener(wl_buffer, &_wl_sw_buffer_listener,
                         wl_surface_get_user_data(context->wl_surface));

  _sw_context_copy_to_buffer(&context->base, (color_t_ *)data, width, height);

  wl_buffer_destroy(context->wl_buffer);
  munmap(context->data, context->size);

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = (color_t_ *)data;
  context->wl_buffer = wl_buffer;
  context->data = data;
  context->size = width * height * COLOR_SIZE;

  return true;
}

void
wl_sw_context_present(
  wl_sw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->wl_buffer != NULL);
  assert(context->wl_surface != NULL);

  wl_surface_attach(context->wl_surface, context->wl_buffer, 0, 0);
  wl_surface_damage(context->wl_surface, 0, 0,
                    context->base.base.width, context->base.base.height);
  wl_surface_commit(context->wl_surface);
}

#else

const int wl_sw_context = 0;

#endif /* HAS_WAYLAND */
