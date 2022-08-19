/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __COLOR_H
#define __COLOR_H

#include <stdint.h>

#define COLOR_SIZE 4

typedef struct color_t {
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
} color_t_;

#define color(_a,_r,_g,_b) \
  ((color_t_){ .b = (uint8_t)(_b), .g = (uint8_t)(_g), \
               .r = (uint8_t)(_r), .a = (uint8_t)(_a) })

#define color_black \
  ((color_t_){ .b = 0, .g = 0, .r = 0, .a = 255 })

#define color_transparent_black \
  ((color_t_){ .b = 0, .g = 0, .r = 0, .a = 0 })

#define color_white \
  ((color_t_){ .b = 255, .g = 255, .r = 255, .a = 255 })

#define color_transparent_white \
  ((color_t_){ .b = 255, .g = 255, .r = 255, .a = 0})

#define color_of_int(_c) \
  ((color_t_){ .b = ((_c) >> 0) & 0xFF, \
               .g = ((_c) >> 8) & 0xFF, \
               .r = ((_c) >> 16) & 0xFF, \
               .a = ((_c) >> 24) & 0xFF })

#define color_to_int(_c) \
  (((_c).a << 24) | ((_c).r << 16) | ((_c).g << 8) | ((_c).b << 0))

#define alpha_blend(_a,_c1,_c2) \
  ((color_t_){ \
    .b = (uint8_t)(((_c1).b * (255 - (_a)) + (_c2).b * (_a)) / 255), \
    .g = (uint8_t)(((_c1).g * (255 - (_a)) + (_c2).g * (_a)) / 255), \
    .r = (uint8_t)(((_c1).r * (255 - (_a)) + (_c2).r * (_a)) / 255), \
    .a = (uint8_t)(((_c1).a * (255 - (_a)) + (_c2).a * (_a)) / 255) })

#endif /* __COLOR_H */
