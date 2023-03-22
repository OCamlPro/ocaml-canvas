/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __CONTEXT_INTERNAL_H
#define __CONTEXT_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct context_t {
  bool offscreen;
  int32_t width;
  int32_t height;
} context_t;

#endif /* __CONTEXT_INTERNAL_H */
