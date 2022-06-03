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
#include <string.h>
#include <memory.h>
#include <math.h>
#include <limits.h>
#include <wchar.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

wchar_t *
mbs_to_wcs(
  const char *mb_text)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, mb_text, -1, NULL, 0);
  if (len == 0) {
    return NULL;
  }
  wchar_t *wc_text = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
  if (wc_text != NULL) {
    int res = MultiByteToWideChar(CP_UTF8, 0, mb_text, -1, wc_text, len + 1);
    if (res == 0) {
      free(wc_text);
      return NULL;
    }
  }
  return wc_text;
}

#else

wchar_t *
mbs_to_wcs(
  const char *mb_text)
{
  size_t len = mbstowcs(NULL, mb_text, 0);
  if (len == 0) {
    return NULL;
  }
  wchar_t *wc_text = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
  if (wc_text != NULL) {
    size_t res = mbstowcs(wc_text, mb_text, len + 1);
    if (res == (size_t)(-1)) {
      free(wc_text);
      return NULL;
    }
  }
  return wc_text;
}

#endif /* _WIN32 || _WIN64 */

uint32_t
decode_utf8_char(
  const char **utf8)
{
  assert(utf8 != NULL);
  assert(*utf8 != NULL);

  static int8_t nb_bytes[256] = {
    // 0xxx xxxx (0)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    // 10xx xxxx (continuation bytes, invalid as first byte)
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

    // 110x xxxx (1)
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

    // 1110 xxxx (2)
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

    // 1111 0xxx (3) (only 1111 00xx and 1111 0100 are valid in this range)
    3, 3, 3, 3, 3, -1, 1, -1,

    // 1111 1xxx and above (invalid)
    -1, -1, -1, -1, -1, -1, -1, -1
  };

  static uint8_t mask[4] = { 0x7F, 0x1F, 0x0F, 0x07 };

  const uint8_t *p = (const uint8_t *)*utf8;

  int8_t n = nb_bytes[*p];

  if (n < 0) {
    *utf8 = (const char *)++p;
    return 0xFFFD; /* Unicode replacement character */
  }

  uint32_t c = (*p++ & mask[n]);

  while (n-- > 0) {
    if ((*p & 0xC0) != 0x80) {
      if (*p != 0) {
        ++p;
      }
      *utf8 = (const char *)p;
      return 0xFFFD;
    }
    c = (c << 6) + (*p++ & 0x3F);
  }

  *utf8 = (const char *)p;

  return c;
}


#if defined(_WIN32) || defined(_WIN64)

char * strndup(const char *s, size_t n)
{
  n = strnlen(s, n);
  char *c = (char *)calloc(n+1, sizeof(char));
  if (c == NULL) {
    return NULL;
  }
  memcpy(c, s, n);
  c[n] = 0;
  return c;
}

#endif
