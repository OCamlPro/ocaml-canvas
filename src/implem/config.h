/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

#include <stdbool.h>
#include <assert.h>

typedef enum os_type_t {
  OS_UNKNOWN = 0,
  OS_WIN32   = 1,
  OS_OSX     = 2,
  OS_UNIX    = 3
} os_type_t;

typedef enum impl_type_t {
  IMPL_NONE    = 0,
  IMPL_CANVAS  = 1,
  IMPL_GDI     = 2,
  IMPL_QUARTZ  = 3,
  IMPL_X11     = 4,
  IMPL_WAYLAND = 5
} impl_type_t;

#define switch_IMPL() switch (get_impl_type())

#ifdef HAS_GDI
#define case_GDI(s) case IMPL_GDI: { s; } break
#else
#define case_GDI(s)
#endif

#ifdef HAS_QUARTZ
#define case_QUARTZ(s) case IMPL_QUARTZ: { s; } break
#else
#define case_QUARTZ(s)
#endif

#ifdef HAS_X11
#define case_X11(s) case IMPL_X11: { s; } break
#else
#define case_X11(s)
#endif

#ifdef HAS_WAYLAND
#define case_WAYLAND(s) case IMPL_WAYLAND: { s; } break
#else
#define case_WAYLAND(s)
#endif

#define default_fail() default: assert(!"Missing implementation"); break
#define default_ignore() default: break

void set_impl_type(impl_type_t it);
impl_type_t get_impl_type();

os_type_t get_os_type();

#endif /* __CONFIG_H */
