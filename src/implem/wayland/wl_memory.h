/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __WL_MEMORY_H
#define __WL_MEMORY_H

#include "stdint.h"

#include <wayland-client.h>

struct wl_buffer* wl_create_buffer(
    uint32_t width,
    uint32_t height,
    uint8_t **data
);

#endif /*__WL_MEMORY_H*/