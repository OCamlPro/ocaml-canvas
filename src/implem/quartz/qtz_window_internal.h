/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_WINDOW_INTERNAL_H
#define __QTZ_WINDOW_INTERNAL_H

#include <Cocoa/Cocoa.h>

#include "../window_internal.h"

typedef struct qtz_window_t {

  /* Common to all windows */
  window_t base;

  /* Specific to Quartz windows */
  NSWindow *nswin;
  NSView *nsview;

} qtz_window_t;

#endif /* __QTZ_WINDOW_INTERNAL_H */
