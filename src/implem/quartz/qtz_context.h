/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_CONTEXT_H
#define __QTZ_CONTEXT_H

#include <stdint.h>
#include <stdbool.h>

#include "../color.h"
#include "../context_internal.h"
#include "qtz_target.h"

typedef struct qtz_context_t qtz_context_t;

qtz_context_t *
qtz_context_create(
  qtz_target_t *target,
  int32_t width,
  int32_t height);

void
qtz_context_destroy(
  qtz_context_t *context);

bool
qtz_context_resize(
  qtz_context_t *context,
  int32_t width,
  int32_t height);

void
qtz_context_present(
  qtz_context_t *context);

#endif /* __QTZ_CONTEXT_H */
