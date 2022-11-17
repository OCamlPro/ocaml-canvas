/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __UNICODE_H
#define __UNICODE_H

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>

wchar_t *
mbs_to_wcs(
  const char *mb_text);

uint32_t
decode_utf8_char(
  const char **utf8);

#if defined(_WIN32) || defined(_WIN64)

char * strndup(const char *s, size_t n);

#endif

#endif /* __UNICODE_H */
