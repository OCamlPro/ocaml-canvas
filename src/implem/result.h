/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __RESULT_H
#define __RESULT_H

#include <stdbool.h>
#include <assert.h>

#define define_result_type(res_type,err_type) \
typedef struct result_##res_type##_##err_type##_t { \
  res_type value; \
  err_type error; \
  bool is_error; \
} result_##res_type##_##err_type##_t

#define result_t(res_type,err_type) result_##res_type##_##err_type##_t

#define ok(res_type,err_type,v) \
  ((result_##res_type##_##err_type##_t){ .value = (v), .is_error = false })
#define error(res_type,err_type,e) \
  ((result_##res_type##_##err_type##_t){ .error = (e), .is_error = true })
#define res_value(r,d) ((r).is_error ? (d) : (r).value)
#define get_ok(r) (assert(!(r).is_error), (r).value)
#define get_error(r) (assert((r).is_error), (r).error)
#define is_ok(r) (!(r).is_error)
#define is_error(r) ((r).is_error)

#endif /* __RESULT_H */
