/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t hash_t;

typedef hash_t key_hash_fun_t(const void *key);
typedef bool key_equal_fun_t(const void *key1, const void *key2);

typedef struct hashtable_t hashtable_t;
typedef struct hashtable_iterator_t hashtable_iterator_t;

hashtable_t *
ht_new(
  key_hash_fun_t *key_hash,
  key_equal_fun_t *key_equal,
  size_t size);

void
ht_delete(
  hashtable_t *ht);

bool
ht_add(
  hashtable_t *ht,
  const void *key,
  void *val);

bool
ht_remove(
  hashtable_t *ht,
  const void *key);

void *
ht_find(
  const hashtable_t *ht,
  const void *key);

hashtable_iterator_t *
ht_get_iterator(
  hashtable_t *ht);

void
ht_free_iterator(
  hashtable_iterator_t *i);

void *
ht_iterator_next(
  hashtable_iterator_t *i);

#endif /* __HASHTABLE_H */
