/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __TUPLES_H
#define __TUPLES_H

#include <stdint.h>

#define _define_pair_type(type) \
typedef struct pair_##type##_t { \
  type m1; \
  type m2; \
} pair_##type##_t

_define_pair_type(int32_t);
_define_pair_type(uint32_t);
_define_pair_type(double);

#define pair_t(type) pair_##type##_t

#define pair(type,p1,p2) ((pair_##type##_t){ .m1 = (p1), .m2 = (p2) })
#define fst(p) ((p).m1)
#define snd(p) ((p).m2)

#endif /* __TUPLES_H */
