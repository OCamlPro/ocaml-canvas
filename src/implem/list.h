/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/


#ifndef __LIST_H
#define __LIST_H

#include <stdbool.h>

typedef struct list_t list_t;
typedef struct list_iterator_t list_iterator_t;

typedef void free_val_fun_t(void *entry);

list_t *
list_new(
  free_val_fun_t *free_val);

void
list_delete(
  list_t *l);

void
list_reset(
  list_t *l);

bool
list_push(
  list_t *l,
  void *val);

void *
list_pop(
  list_t *l);

void *
list_peek(
  list_t *l);

void
list_rev(
  list_t *l);

bool
list_is_empty(
  const list_t *l);

list_iterator_t *
list_get_iterator(
  list_t *l);

void
list_free_iterator(
  list_iterator_t *i);

void *
list_iterator_next(
  list_iterator_t *i);

#endif /* __LIST_H */
