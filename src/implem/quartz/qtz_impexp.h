/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __QTZ_IMPEXP_H
#define __QTZ_IMPEXP_H

#include <stdint.h>
#include <stdbool.h>

#include "../pixmap.h"

bool
qtz_impexp_init(
  void);

void
qtz_impexp_terminate(
  void);

bool
qtz_impexp_export_png(
  const pixmap_t *pixmap,
  const char *filename);

bool
qtz_impexp_import_png(
  pixmap_t *pixmap,
  int32_t dx,
  int32_t dy,
  const char *filename);

#endif /* __QTZ_IMPEXP_H */
