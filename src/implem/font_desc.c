/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "unicode.h"
#include "font_desc.h"
#include "font_desc_internal.h"

#define MAX_FAMILY_SIZE 256

font_desc_t *
font_desc_create(
  void)
{
  font_desc_t *fd = (font_desc_t *)calloc(1, sizeof(font_desc_t));
  if (fd == NULL) {
    return NULL;
  }

  font_desc_reset(fd);

  return fd;
}

void
font_desc_destroy(
  font_desc_t *fd)
{
  assert(fd != NULL);

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  free(fd);
}

void
font_desc_reset(
  font_desc_t *fd)
{
  assert(fd != NULL);

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  fd->family = NULL;
  fd->size = 0.0;
  fd->slant = SLANT_ROMAN;
  fd->weight = 400.0;
  fd->scale = 1.0;
}

font_desc_t *
font_desc_copy(
  const font_desc_t *fd)
{
  assert(fd != NULL);

  font_desc_t *fdc = (font_desc_t *)calloc(1, sizeof(font_desc_t));
  if (fdc == NULL) {
    return NULL;
  }

  if (fd->family != NULL) {
    fdc->family = strndup(fd->family, MAX_FAMILY_SIZE);
    if (fdc->family == NULL) {
      font_desc_destroy(fdc);
      return NULL;
    }
  } else {
    fdc->family = NULL;
  }

  fdc->size = fd->size;
  fdc->slant = fd->slant;
  fdc->weight = fd->weight;
  fdc->scale = fd->scale;

  return fdc;
}

bool
font_desc_equal(
  const font_desc_t *fd1,
  const font_desc_t *fd2)
{
  assert(fd1 != NULL);
  assert(fd2 != NULL);

  return
    ((fd1->family != NULL && fd2->family != NULL &&
      strncmp(fd1->family, fd2->family, MAX_FAMILY_SIZE) == 0) ||
     (fd1->family == NULL && fd2->family == NULL)) &&
    fd1->size == fd2->size &&
    fd1->slant == fd2->slant &&
    fd1->weight == fd2->weight &&
    fd1->scale == fd2->scale;
}

bool
font_desc_is_set(
  const font_desc_t *fd)
{
  assert (fd != NULL);

  return fd->family != NULL;
}

bool
font_desc_set(
  font_desc_t *fd,
  const char *family,
  double size,
  font_slant_t slant,
  int32_t weight)
{
  assert(fd != NULL);
  assert(family != NULL);
  assert(size > 0.0);
  assert(weight >= 0);

  family = strndup(family, MAX_FAMILY_SIZE);
  if (family == NULL) {
    return false;
  }

  if (fd->family != NULL) {
    free((char *)fd->family);
  }

  fd->family = family;
  fd->size = size;
  fd->slant = slant;
  fd->weight = weight;
  fd->scale = 1.0;

  return true;
}

void
font_desc_scale(
  font_desc_t *fd,
  double s)
{
  assert(fd != NULL);

  fd->scale = s;
}

