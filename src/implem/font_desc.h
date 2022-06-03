/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FONT_DESC_H
#define __FONT_DESC_H

#include <stdint.h>
#include <stdbool.h>

typedef struct font_desc_t font_desc_t;

typedef enum font_slant_t {
  SLANT_ROMAN   = 0,
  SLANT_ITALIC  = 1,
  SLANT_OBLIQUE = 2
} font_slant_t;

font_desc_t *
font_desc_create();

void
font_desc_destroy(
  font_desc_t *fd);

void
font_desc_reset(
  font_desc_t *fd);

font_desc_t *
font_desc_copy(
  const font_desc_t *fd);

bool
font_desc_equal(
  const font_desc_t *fd1,
  const font_desc_t *fd2);

bool
font_desc_is_set(
  const font_desc_t *fd);

bool
font_desc_set(
  font_desc_t *fd,
  const char *family,
  double size,
  font_slant_t slant,
  int32_t weight);

void
font_desc_scale(
  font_desc_t *fd,
  double s);

#endif /* __FONT_DESC_H */
