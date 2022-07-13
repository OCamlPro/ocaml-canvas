/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <locale.h>

#include "config.h"
#include "hashtable.h"
#include "event.h"
#include "window.h"
#include "surface.h"
#include "canvas.h"
#include "canvas_internal.h"
#include "poly_render.h"
#include "impexp.h"

#ifdef HAS_GDI
#include "gdi/gdi_backend.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_backend.h"
#endif
#ifdef HAS_X11
#include "x11/x11_backend.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_backend.h"
#endif

static hashtable_t *_backend_id_to_canvas = NULL;

static bool
_backend_process_event(
  event_t *event,
  event_listener_t *next_listener) // next listener or any other data (closure)
{
  assert(event != NULL);

  bool result = false;

  window_t *window = (window_t *)event->target;
  canvas_t *canvas = (canvas_t *)window_get_data(window);

  /* Safeguard against early events (i.e. events generated during
     window creation, before the canvas is fully created) or late
     events (those generated while the canvas is being destroyed) */
  if (canvas == NULL) {
    return false;
  }

  event->target = (void *)canvas;

  switch (event->type) {
    case EVENT_PRESENT: /* internal event */
      surface_present(canvas->surface, &event->desc.present.data);
      result = true;
      break;
    case EVENT_RESIZE:
// Ensure width/height > 0
      _canvas_set_size_internal(canvas,
                                event->desc.resize.width,
                                event->desc.resize.height);
      result = event_notify(next_listener, event);
      break;
    case EVENT_CLOSE:
      event_notify(next_listener, event);
      canvas_destroy(canvas);
      result = false; // true to prevent close ?
      break;
    default:
      result = event_notify(next_listener, event);
      break;
  }

  event->target = (void *)window;

  return result;
}

static event_listener_t
_backend_event_listener = {
  .process_event = _backend_process_event,
  .next_listener = NULL
};

static hash_t
_backend_id_hash(
  const int32_t *id)
{
  return (hash_t)(uintptr_t)*id;
}

static bool
_backend_id_equal(
  const int32_t *id1,
  const int32_t *id2)
{
  return (*id1) == (*id2);
}

bool
backend_init(
  impl_type_t impl_type)
{
  assert(impl_type != IMPL_NONE);

  if (get_impl_type() != IMPL_NONE) {
    return false;
  }

  if (_backend_id_to_canvas == NULL) {
    _backend_id_to_canvas = ht_new((key_hash_fun_t *)_backend_id_hash,
                                   (key_equal_fun_t *)_backend_id_equal, 1024);
    if (_backend_id_to_canvas == NULL) {
      return false;
    }
  }

  bool result = false;

  setlocale(LC_ALL, "");

  switch (impl_type) {
    case_GDI(result = gdi_backend_init());
    case_QUARTZ(result = qtz_backend_init());
    case_X11(result = x11_backend_init());
    case_WAYLAND(/*result = wl_backend_init()*/);
    default_ignore();
  }

  if (result == true) {
    set_impl_type(impl_type);
    poly_render_init();
    impexp_init();

    switch_IMPL() {
      case_GDI(gdi_backend_set_listener(&_backend_event_listener));
      case_QUARTZ(qtz_backend_set_listener(&_backend_event_listener));
      case_X11(x11_backend_set_listener(&_backend_event_listener));
      case_WAYLAND(wl_backend_set_listener(&_backend_event_listener));
      default_fail();
    }
  }

  return result;
}

void
backend_terminate(
  void)
{
  if (get_impl_type() == IMPL_NONE) {
    return;
  }

  switch_IMPL() {
    case_GDI(gdi_backend_terminate());
    case_QUARTZ(qtz_backend_terminate());
    case_X11(x11_backend_terminate());
    case_WAYLAND(wl_backend_terminate());
    default_fail();
  }

  impexp_terminate();
  ht_delete(_backend_id_to_canvas);
  set_impl_type(IMPL_NONE);
}

void
backend_run(
  event_listener_t *event_listener)
{
  assert(event_listener != NULL);
  assert(event_listener->process_event != NULL);

  if (get_impl_type() == IMPL_NONE) {
    return;
  }

  /* If already running, ignore */
  if (_backend_event_listener.next_listener != NULL) {
    return;
  }

  _backend_event_listener.next_listener = event_listener;

  switch_IMPL() {
    case_GDI(gdi_backend_run());
    case_QUARTZ(qtz_backend_run());
    case_X11(x11_backend_run());
    case_WAYLAND(wl_backend_run());
    default_fail();
  }

  _backend_event_listener.next_listener = NULL;
}

void
backend_stop(
  void)
{
  switch_IMPL() {
    case_GDI(gdi_backend_stop());
    case_QUARTZ(qtz_backend_stop());
    case_X11(x11_backend_stop());
    case_WAYLAND(wl_backend_stop());
    default_ignore();
  }
}

int32_t
backend_next_id(
  void)
{
  static int32_t id = 0;
  int32_t old_id = id;

  do {
    ++id;
    if (id > 0x3FFFFFFF) {
      id = 1;
    }
    /* Exhausted ids (unlikely) */
    if (id == old_id) {
      return 0;
    }
  } while (ht_find(_backend_id_to_canvas, (void *)&id) != NULL);

  return id;
}

void
backend_add_canvas(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->id != 0);

  ht_add(_backend_id_to_canvas, (void *)&(canvas->id), (void *)canvas);
}

void
backend_remove_canvas(
  canvas_t *canvas)
{
  assert(canvas != NULL);
  assert(canvas->id != 0);

  ht_remove(_backend_id_to_canvas, (void *)&(canvas->id));
}

canvas_t *
backend_get_canvas(
  int32_t id)
{
  return (canvas_t *)ht_find(_backend_id_to_canvas, (void *)&id);
}
