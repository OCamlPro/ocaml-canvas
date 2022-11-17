/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GDI_IMPEXP_H
#define __GDI_IMPEXP_H

#include <stdint.h>
#include <stdbool.h>

#include "../pixmap.h"

bool
gdi_impexp_init(
  void);

void
gdi_impexp_terminate(
  void);

bool
gdi_impexp_export_png(
  const pixmap_t *pixmap,
  const char *filename);

bool
gdi_impexp_import_png(
  pixmap_t *pixmap,
  int32_t dx,
  int32_t dy,
  const char *filename);

#endif /* __GDI_IMPEXP_H */
