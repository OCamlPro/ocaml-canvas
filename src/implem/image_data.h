/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __IMAGE_DATA_H
#define __IMAGE_DATA_H

#include <stdint.h>

#include "color.h"

typedef struct image_data_t {
  color_t_ *data;
  int32_t width;
  int32_t height;
} image_data_t;

#define image_data(_w,_h,_d) \
  ((image_data_t){ .width = (_w), .height = (_h), \
                   .data = ((_d) != NULL) ? (_d) : \
                             calloc((_w) * (_h), COLOR_SIZE) })

#define image_data_destroy(_id) \
  do { \
    if ((_id).data != NULL) { \
      free((_id).data); \
      (_id).data = NULL; \
    } \
  } while (0)

#define image_data_valid(_id) \
  (((_id).data != NULL) && ((_id).width > 0) && ((_id).height > 0))

#endif /* __IMAGE_DATA_H */
