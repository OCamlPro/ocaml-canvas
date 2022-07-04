/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __IMPEXP_H
#define __IMPEXP_H

#include <stdbool.h>
#include <stdint.h>

#include "color.h"

bool
impexp_init(
  void);

void
impexp_terminate(
  void);

bool
impexp_export_png(
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename);

bool
impexp_import_png(
  color_t_ **p_data,
  int32_t *p_width,
  int32_t *p_height,
  int32_t x,
  int32_t y,
  const char *filename);

#endif /* __IMPEXP_H */
