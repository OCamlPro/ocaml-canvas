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

#include <stdbool.h>
#include <stdint.h>

#include "../color.h"

bool
gdi_impexp_init(
  void);

void
gdi_impexp_terminate(
  void);

bool
gdi_impexp_export_png(
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename);

bool
gdi_impexp_import_png(
  color_t_ **p_data,
  int32_t *p_width,
  int32_t *p_height,
  int32_t dx,
  int32_t dy,
  const char *filename);

#endif /* __GDI_IMPEXP_H */
