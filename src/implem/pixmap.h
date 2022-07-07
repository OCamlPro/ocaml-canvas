/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __PIXMAP_H
#define __PIXMAP_H

#include <stdint.h>

#include "util.h"
#include "color.h"

typedef struct pixmap_t {
  color_t_ *data;
  int32_t width;
  int32_t height;
} pixmap_t;

#define pixmap(w,h,d) \
  ((pixmap_t){ .width = (w), .height = (h), \
               .data = ((d) != NULL) ? (d) : \
                        calloc((w) * (h), COLOR_SIZE) })

#define pixmap_copy(p) \
  ((pixmap_t){ .width = (p).width, .height = (p).height, \
               .data = ((p).data == NULL) ? NULL : \
                        memdup((p).data, (p).width * (p).height * COLOR_SIZE) })

#define pixmap_destroy(p) \
  do { \
    if ((p).data != NULL) { \
      free((p).data); \
      (p).data = NULL; \
    } \
  } while (0)

#define pixmap_valid(p) \
  (((p).data != NULL) && ((p).width > 0) && ((p).height > 0))

#define pixmap_at(p,i,j) \
  ((p).data[(i) * (p).width + (j)])

void
pixmap_blit(
  pixmap_t *dp,
  int32_t dx,
  int32_t dy,
  const pixmap_t *sp,
  int32_t sx,
  int32_t sy,
  int32_t width,
  int32_t height);

#endif /* __PIXMAP_H */
