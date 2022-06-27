/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_KEYBOARD_H
#define __WL_KEYBOARD_H
#include "../event.h"


key_code_t
wl_translate_sym_to_keycode(
    uint8_t sym
);

#endif /* __WL_KEYBOARD_H */