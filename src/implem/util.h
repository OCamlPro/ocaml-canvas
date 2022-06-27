/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>
#include <wchar.h>

#define swap(t,a,b) do { \
  t tmp = (a); \
  (a) = (b); \
  (b) = tmp; \
} while (0)

#undef min
#undef max
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

#define min4(a,b,c,d) (min(min(a,b),min(c,d)))
#define max4(a,b,c,d) (max(max(a,b),max(c,d)))

#define between(x,l,h) (((l) < (x)) && ((x) < (h)))

#define clip(a,b,c) ((a) < (b) ? (b) : (a) > (c) ? (c) : b)

int16_t
clip_i32_to_i16(
  int32_t i);

uint16_t
clip_ui32_to_ui16(
  uint32_t i);

int
numbits(
  uint64_t i);

int32_t
fastround(
  double d);

// Normalize the angle a between [0; 2Pi[
double
normalize_angle(
  double a);

#define adjust_blit_info(dwidth,dheight,dx,dy,swidth,sheight,sx,sy,width,height) \
  do { \
    if ((dx) < 0) { (sx) -= (dx); (width)  += (dx); (dx) = 0; } \
    if ((dy) < 0) { (sy) -= (dy); (height) += (dy); (dy) = 0; } \
    if ((sx) < 0) { (dx) -= (sx); (width)  += (sx); (sx) = 0; } \
    if ((sy) < 0) { (dy) -= (sy); (height) += (sy); (sy) = 0; } \
    if ((dx) + (width)  > (dwidth))  { width  = (dwidth)  - (dx); } \
    if ((dy) + (height) > (dheight)) { height = (dheight) - (dy); } \
    if ((sx) + (width)  > (swidth))  { width  = (swidth)  - (sx); } \
    if ((sy) + (height) > (sheight)) { height = (sheight) - (sy); } \
  } while (0)

void *
memdup(
  void *p,
  size_t size);

#endif /* __UTIL_H */
