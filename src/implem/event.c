/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

#include "event.h"

bool
event_notify(
  event_listener_t *event_listener,
  event_t *event)
{
  assert(event != NULL);

  if (event_listener) {
    assert(event_listener->process_event != NULL);
    return event_listener->process_event(event, event_listener->next_listener);
  } else {
    return false;
  }
}
