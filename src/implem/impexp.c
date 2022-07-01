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
#include "color.h"

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
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename)
{
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);
  assert(filename != NULL);

  bool res = false;

  switch_IMPL() {
    case_GDI(res = gdi_impexp_export_png(data, width, height, filename));
    case_QUARTZ(res = qtz_impexp_export_png(data, width, height, filename));
    case_X11(res = unx_impexp_export_png(data, width, height, filename));
    case_WAYLAND(res = unx_impexp_export_png(data, width, height, filename));
    default_fail();
  }

  return res;
}
