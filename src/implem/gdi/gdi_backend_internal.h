/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_BACKEND_INTERNAL_H
#define __GDI_BACKEND_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include <windows.h>

#include "../hashtable.h"
#include "../event.h"
#include "gdi_window.h"

typedef struct gdi_backend_t {
  hashtable_t *hwnd_to_win;

  HINSTANCE hinst;
  WCHAR *class_framed;
  WCHAR *class_frameless;

  bool running;
  bool modal_op;
  UINT_PTR modal_timer;

  event_listener_t *listener;

  double musec_per_tick;
  int64_t next_frame;

} gdi_backend_t;

extern gdi_backend_t *gdi_back;

#endif /* __GDI_BACKEND_INTERNAL_H */
