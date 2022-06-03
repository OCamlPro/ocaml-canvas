/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_TARGET_H
#define __QTZ_TARGET_H

#include <Cocoa/Cocoa.h>

typedef struct qtz_target_t {
  NSView *nsview;
} qtz_target_t;

qtz_target_t *
qtz_target_create(
  NSView *nsview);

void
qtz_target_destroy(
  qtz_target_t *target);

#endif /* __QTZ_TARGET_H */
