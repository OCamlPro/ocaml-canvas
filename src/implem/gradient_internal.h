/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __GRADIENT_INTERNAL_H
#define __GRADIENT_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#include "object.h"
#include "color.h"

typedef struct gradient_node_t {
  double pos;
  color_t_ color;
  struct gradient_node_t *next;
} gradient_node_t;

typedef struct linear_gradient_t {
  double pos1_x, pos2_x, pos1_y, pos2_y;
} linear_gradient_t;

typedef struct radial_gradient_t {
  double pos1_x, pos2_x, pos1_y, pos2_y, r1, r2;
} radial_gradient_t;

typedef union gradient_properties_t {
  linear_gradient_t linear;
  radial_gradient_t radial;
} gradient_properties_t;

typedef struct gradient_t {
  INHERITS_OBJECT;
  bool is_linear;
  gradient_properties_t properties;

  //TODO : This is now a linked list.
  // Test linked list vs array with binary search
  gradient_node_t *nodes;

  void *data; // user data (put in obj base ?)
} gradient_t;

#endif /* __GRADIENT_INTERNAL_H */
