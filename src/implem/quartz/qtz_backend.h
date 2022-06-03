/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_BACKEND_H
#define __QTZ_BACKEND_H

#include <stdint.h>
#include <stdbool.h>

#include <Cocoa/Cocoa.h>

#include "../event.h"
#include "qtz_window.h"

typedef struct qtz_backend_t qtz_backend_t;

int64_t
qtz_get_time(
  void);

bool
qtz_backend_init(
  void);

void
qtz_backend_terminate(
  void);

void
qtz_backend_add_window(
  qtz_window_t *w);

void
qtz_backend_remove_window(
  const qtz_window_t *w);

qtz_window_t *
qtz_backend_get_window(
  const NSWindow *nswin);

void
qtz_backend_set_listener(
  event_listener_t *listener);

event_listener_t *
qtz_backend_get_listener(
  void);

void
qtz_backend_run(
  void);

void
qtz_backend_stop(
  void);

#endif /* __QTZ_BACKEND_H */
