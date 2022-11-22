/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_BACKEND_INTERNAL_H
#define __WL_BACKEND_INTERNAL_H

#include <stdbool.h>

#include "../event.h"

typedef struct wl_backend_t {

  bool running;
  event_listener_t *listener;

} wl_backend_t;

extern wl_backend_t *wl_back;

#endif /* __WL_BACKEND_INTERNAL_H */
