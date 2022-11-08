/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WINDOW_INTERNAL_H
#define __WINDOW_INTERNAL_H

#include <stdbool.h>
#include <stdint.h>

typedef struct window_t {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
  bool decorated;
  bool resizeable;
  bool visible;
  void *data;
} window_t;

#endif /* __WINDOW_INTERNAL_H */
