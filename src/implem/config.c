/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdbool.h>
#include <assert.h>

#include "config.h"

static impl_type_t impl_type = IMPL_NONE;

void
set_impl_type(
  impl_type_t it)
{
  assert(it != IMPL_CANVAS);
#ifndef HAS_GDI
  assert(it != IMPL_GDI);
#endif
#ifndef HAS_QUARTZ
  assert(it != IMPL_QUARTZ);
#endif
#ifndef HAS_X11
  assert(it != IMPL_X11);
#endif
#ifndef HAS_WAYLAND
  assert(it != IMPL_WAYLAND);
#endif
  impl_type = it;
}

impl_type_t
get_impl_type()
{
  return impl_type;
}

os_type_t
get_os_type()
{
#if defined(_WIN32) || defined(_WIN64)
  return OS_WIN32;
#elif defined(__APPLE__) && defined(__MACH__)
  return OS_OSX;
#else
  return OS_UNIX;
#endif
}
