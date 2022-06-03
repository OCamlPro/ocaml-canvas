/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PRESENT_DATA_H
#define __PRESENT_DATA_H

#include "config.h"

#ifdef HAS_GDI
#include "gdi/gdi_present_data.h"
#endif
#ifdef HAS_QUARTZ
#include "quartz/qtz_present_data.h"
#endif
#ifdef HAS_X11
#include "x11/x11_present_data.h"
#endif
#ifdef HAS_WAYLAND
#include "wayland/wl_present_data.h"
#endif

typedef union present_data_t {
#ifdef HAS_GDI
  gdi_present_data_t gdi;
#endif
#ifdef HAS_QUARTZ
  qtz_present_data_t qtz;
#endif
#ifdef HAS_X11
  x11_present_data_t x11;
#endif
#ifdef HAS_WAYLAND
  wl_present_data_t wl;
#endif
} present_data_t;

#endif /* __PRESENT_DATA_H */
