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
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "config.h"
#include "pixmap.h"

#ifdef HAS_GDI
#include "gdi/gdi_impexp.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_impexp.h"
#endif
#if defined HAS_X11 || defined HAS_WAYLAND
#include "unix/unx_impexp.h"
#endif

bool
impexp_init(
  void)
{
  bool res = false;

  switch_IMPL() {
    case_GDI(res = gdi_impexp_init());
    case_QUARTZ(res = qtz_impexp_init());
    case_X11(res = unx_impexp_init());
    case_WAYLAND(res = unx_impexp_init());
    default_fail();
  }

  return res;
}

void
impexp_terminate(
  void)
{
  switch_IMPL() {
    case_GDI(gdi_impexp_terminate());
    case_QUARTZ(qtz_impexp_terminate());
    case_X11(unx_impexp_terminate());
    case_WAYLAND(unx_impexp_terminate());
    default_fail();
  }
}

bool
impexp_export_png(
  const pixmap_t *pixmap,
  const char *filename)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap));
  assert(filename != NULL);

  bool res = false;

  switch_IMPL() {
    case_GDI(res = gdi_impexp_export_png(pixmap, filename));
    case_QUARTZ(res = qtz_impexp_export_png(pixmap, filename));
    case_X11(res = unx_impexp_export_png(pixmap, filename));
    case_WAYLAND(res = unx_impexp_export_png(pixmap, filename));
    default_fail();
  }

  return res;
}

bool
impexp_import_png(
  pixmap_t *pixmap,
  int32_t x,
  int32_t y,
  const char *filename)
{
  assert(pixmap != NULL);
  assert(filename != NULL);

  bool res = false;

  switch_IMPL() {
    case_GDI(res = gdi_impexp_import_png(pixmap, x, y, filename));
    case_QUARTZ(res = qtz_impexp_import_png(pixmap, x, y, filename));
    case_X11(res = unx_impexp_import_png(pixmap, x, y, filename));
    case_WAYLAND(res = unx_impexp_import_png(pixmap, x, y, filename));
    default_fail();
  }

  return res;
}
