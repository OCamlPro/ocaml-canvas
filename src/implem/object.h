/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __OBJECT_H
#define __OBJECT_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef struct object_t {
  void *data;
  int32_t count;
} object_t;

#define INHERITS(type)                                                        \
  type base_##type

#define INHERITS_OBJECT                                                       \
  object_t base_object_t

#define DECLARE_OBJECT_METHODS(type,prefix)                                   \
type * prefix##_retain(type *o);                                              \
void prefix##_release(type *o);                                               \
void prefix##_set_data(type *o, void *data);                                  \
void * prefix##_get_data(type *o);                                            \

#define IMPLEMENT_OBJECT_METHODS(type,prefix,destroy)                         \
                                                                              \
static type * prefix##_alloc(void)                                            \
{                                                                             \
  type *o = (type *)malloc(sizeof(type));                                     \
  if (o != NULL) {                                                            \
    ((object_t *)o)->data = NULL;                                             \
    ((object_t *)o)->count = 1;                                               \
  }                                                                           \
  return o;                                                                   \
}                                                                             \
                                                                              \
type * prefix##_retain(type *o)                                               \
{                                                                             \
  assert(o != NULL);                                                          \
  assert((((object_t *)o)->count) > 0);                                       \
                                                                              \
  ++((object_t *)o)->count;                                                   \
  return o;                                                                   \
}                                                                             \
                                                                              \
static void destroy(type *o);                                                 \
                                                                              \
void prefix##_release(type *o)                                                \
{                                                                             \
  assert(o != NULL);                                                          \
  assert((((object_t *)o)->count) > 0);                                       \
                                                                              \
  if (--(((object_t *)o)->count) == 0) {                                      \
    destroy(o);                                                               \
  }                                                                           \
}                                                                             \
                                                                              \
void prefix##_set_data(type *o, void *data)                                   \
{                                                                             \
  assert(o != NULL);                                                          \
                                                                              \
  ((object_t *)o)->data = data;                                               \
}                                                                             \
                                                                              \
void * prefix##_get_data(type *o)                                             \
{                                                                             \
  assert(o != NULL);                                                          \
  assert(((object_t *)o)->count >= 0);                                        \
                                                                              \
  return ((object_t *)o)->data;                                               \
}

#endif /* __OBJECT_H */
