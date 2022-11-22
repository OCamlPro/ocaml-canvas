/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_TARGET_H
#define __WL_TARGET_H

typedef struct wl_target_t {
  void *dummy;
} wl_target_t;

wl_target_t *
wl_target_create(
  void *dummy);

void
wl_target_destroy(
  wl_target_t *target);

#endif /* __WL_TARGET_H */
