/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_QUARTZ

#include <stdlib.h>
#include <assert.h>

#include <Cocoa/Cocoa.h>

#include "qtz_target.h"

qtz_target_t *
qtz_target_create(
  NSWindow *nswin)
{
  assert(nsview != NULL);
  qtz_target_t *target = (qtz_target_t *)calloc(1, sizeof(qtz_target_t));
  if (target == NULL) {
    return NULL;
  }
  target->nswin = nswin; // technically, should retain/release,
  return target;         // but we're using target before the
}                        // window can be destroyed

void
qtz_target_destroy(
  qtz_target_t *target)
{
  assert(target != NULL);
  free(target);
}

#else

const int qtz_target = 0;

#endif /* HAS_QUARTZ */
