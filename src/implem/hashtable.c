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
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "hashtable.h"

typedef struct hashtable_entry_t hashtable_entry_t;

typedef struct hashtable_entry_t {
  const void *key;
  void *val;
  hashtable_entry_t *next;
} hashtable_entry_t;

typedef struct hashtable_t {
  key_hash_fun_t *key_hash;
  key_equal_fun_t *key_equal;
  hashtable_entry_t **data;
  size_t size;
} hashtable_t;

typedef struct hashtable_iterator_t {
  hashtable_t *hashtable;
  hashtable_entry_t *entry;
  hash_t current_hash;
} hashtable_iterator_t;

hashtable_t *
ht_new(
  key_hash_fun_t *key_hash,
  key_equal_fun_t *key_equal,
  size_t size)
{
  assert(key_hash != NULL);
  assert(key_equal != NULL);
  assert(size > 0);
  hashtable_t *ht = (hashtable_t *)calloc(1, sizeof(hashtable_t));
  if (ht == NULL) {
    return NULL;
  }
  ht->data = (hashtable_entry_t **)calloc(size, sizeof(hashtable_entry_t *));
  if (ht->data == NULL) {
    free(ht);
    return NULL;
  }
  ht->key_hash = key_hash;
  ht->key_equal = key_equal;
  ht->size = size;
  return ht;
}

void
ht_delete(
  hashtable_t *ht)
{
  assert(ht != NULL);
  assert(ht->data != NULL);
  for (size_t i = 0; i < ht->size; ++i) {
      hashtable_entry_t *n = ht->data[i];
      hashtable_entry_t *e;
      while ((e = n)) {
        n = e->next;
        free(e);
      }
  }
  free(ht->data);
  free(ht);
}

static hash_t
_ht_key_hash(
  const hashtable_t *ht,
  const void *key)
{
  return ht->key_hash(key) % ht->size;
}

static hash_t
_ht_key_equal(
  const hashtable_t *ht,
  const void *key1,
  const void *key2)
{
  return ht->key_equal(key1, key2);
}

bool
ht_add(
  hashtable_t *ht,
  const void *key,
  void *val)
{
  assert(ht != NULL);
  assert(ht->data != NULL);
  hash_t h = _ht_key_hash(ht, key);
  hashtable_entry_t **p = &(ht->data[h]);
  hashtable_entry_t *e;
  while ((e = *p)) {
    if (_ht_key_equal(ht, e->key, key)) {
      e->val = val;
      return false;
    } else {
      p = &(e->next);
    }
  }
  e = (hashtable_entry_t *)calloc(1, sizeof(hashtable_entry_t));
  if (e == NULL) {
    return false;
  }
  e->key = key;
  e->val = val;
  e->next = NULL;
  *p = e;
  return true;
}

bool
ht_remove(
  hashtable_t *ht,
  const void *key)
{
  assert(ht != NULL);
  assert(ht->data != NULL);
  hash_t h = _ht_key_hash(ht, key);
  hashtable_entry_t **p = &(ht->data[h]);
  hashtable_entry_t *e;
  while ((e = *p)) {
    if (_ht_key_equal(ht, e->key, key)) {
      *p = e->next;
      free(e);
      return true;
    } else {
      p = &(e->next);
    }
  }
  return false;
}

void *
ht_find(
  const hashtable_t *ht,
  const void *key)
{
  assert(ht != NULL);
  assert(ht->data != NULL);
  hash_t h = _ht_key_hash(ht, key);
  hashtable_entry_t *e = ht->data[h];
  while (e) {
    if (_ht_key_equal(ht, e->key, key)) {
      return e->val;
    } else {
      e = e->next;
    }
  }
  return NULL;
}

static void
_ht_iterator_skip_empty(
  hashtable_iterator_t *i)
{
  assert(i != NULL);
  assert(i->hashtable != NULL);
  assert(i->hashtable->size > 0);
  while ((i->entry == NULL) && (i->current_hash < i->hashtable->size - 1)) {
    i->entry = i->hashtable->data[++(i->current_hash)];
  }
}

hashtable_iterator_t *
ht_get_iterator(
  hashtable_t *ht)
{
  assert(ht != NULL);
  assert(ht->data != NULL);
  hashtable_iterator_t *i =
    (hashtable_iterator_t *)calloc(1, sizeof(hashtable_iterator_t));
  i->hashtable = ht;
  i->entry = i->hashtable->data[0];
  _ht_iterator_skip_empty(i);
  return i;
}

void
ht_free_iterator(
  hashtable_iterator_t *i)
{
  assert(i != NULL);
  free(i);
}

void *
ht_iterator_next(
  hashtable_iterator_t *i)
{
  assert(i != NULL);
  void *val = NULL;
  if (i->entry != NULL) {
    val = i->entry->val;
    i->entry = i->entry->next;
    _ht_iterator_skip_empty(i);
  }
  return val;
}
