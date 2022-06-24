/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __OPTION_H
#define __OPTION_H

#include <stdbool.h>
#include <assert.h>

#define define_option_type(type) \
typedef struct option_##type##_t { \
  type value; \
  bool is_empty; \
} option_##type##_t

#define option_t(type) option_##type##_t

#define some(type,v) ((option_##type##_t){ .value = (v), .is_empty = false })
#define none(type) ((option_##type##_t){ .is_empty = true })

#define opt_value(o,d) ((o).is_empty ? (d) : (o).value)
#define get(o) (assert(!(o).is_empty), (o).value)
#define is_some(o) (!(o).is_empty)
#define is_none(o) ((o).is_empty)

#endif /* __OPTION_H */
