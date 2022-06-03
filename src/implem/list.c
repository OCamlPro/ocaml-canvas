/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include "list.h"

typedef struct list_entry_t list_entry_t;

typedef struct list_entry_t {
  void *val;
  list_entry_t *next;
} list_entry_t;

typedef struct list_t {
  list_entry_t *head;
  free_val_fun_t *free_val;
} list_t;

typedef struct list_iterator_t {
  list_entry_t *entry;
} list_iterator_t;

typedef void free_val_fun_t(void *entry);

list_t *
list_new(free_val_fun_t *free_val)
{
  list_t *l = (list_t *)calloc(1, sizeof(list_t));
  if (l == NULL) {
    return NULL;
  }

  l->free_val = free_val;

  return l;
}

void
list_delete(
  list_t *l)
{
  assert(l != NULL);

  list_reset(l);
  free(l);
}

void
list_reset(
  list_t *l)
{
  assert(l != NULL);

  list_entry_t *e = NULL;
  list_entry_t *n = l->head;

  while ((e = n) != NULL) {
    n = e->next;
    if (l->free_val != NULL) {
      l->free_val(e->val);
    }
    free(e);
  }
}

bool
list_push(
  list_t *l,
  void *val)
{
  assert(l != NULL);

  list_entry_t *e = (list_entry_t *)calloc(1, sizeof(list_entry_t));
  if (e == NULL) {
    return false;
  }

  e->val = val;
  e->next = l->head;
  l->head = e;

  return true;
}

void *
list_pop(
  list_t *l)
{
  assert(l != NULL);

  list_entry_t *e = l->head;
  if (e == NULL) {
    return NULL;
  }

  void *val = e->val;
  l->head = e->next;
  free(e);

  return val;
}

void *
list_peek(
  list_t *l)
{
  assert(l != NULL);

  list_entry_t *e = l->head;
  if (e == NULL) {
    return NULL;
  }

  return e->val;
}

void
list_rev(
  list_t *l)
{
  assert(l != NULL);

  list_entry_t *p = NULL;
  list_entry_t *e = NULL;
  list_entry_t *n = l->head;

  while ((e = n) != NULL) {
    n = e->next;
    e->next = p;
    p = e;
  }
  l->head = p;
}

bool
list_is_empty(
  const list_t *l)
{
  assert(l != NULL);

  return (l->head == NULL);
}

list_iterator_t *
list_get_iterator(
  list_t *l)
{
  assert(l != NULL);

  list_iterator_t *i = (list_iterator_t *)calloc(1, sizeof(list_iterator_t *));
  if (i == NULL) {
    return NULL;
  }

  i->entry = l->head;

  return i;
}

void
list_free_iterator(
  list_iterator_t *i)
{
  assert(i != NULL);

  free(i);
}

void *
list_iterator_next(
  list_iterator_t *i)
{
  assert(i != NULL);

  void *val = NULL;

  if (i->entry != NULL) {
    val = i->entry->val;
    i->entry = i->entry->next;
  }

  return val;
}
