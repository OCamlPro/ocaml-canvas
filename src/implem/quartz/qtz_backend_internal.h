/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_BACKEND_INTERNAL_H
#define __QTZ_BACKEND_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include <Carbon/Carbon.h>

#include "../hashtable.h"
#include "../event.h"

typedef struct qtz_backend_t {

  hashtable_t *nswin_to_win;

  const UCKeyboardLayout *layout;

  bool running;

  event_listener_t *listener;

  uint32_t musec_per_tick_numer;
  uint32_t musec_per_tick_denom;

} qtz_backend_t;

extern qtz_backend_t *qtz_back;

#endif /* __QTZ_BACKEND_INTERNAL_H */
