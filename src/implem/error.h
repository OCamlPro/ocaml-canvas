/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __ERROR_H
#define __ERROR_H

typedef enum error_t {
  ERROR_NONE = 0;
  ERROR_GENERIC = 1;
  ERROR_OUT_OF_MEMORY = 2;
} error_t;

#endif /* __ERROR_H */
