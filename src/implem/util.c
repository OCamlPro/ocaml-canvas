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

int16_t
clip_i32_to_i16(
  int32_t i)
{
  if (i < INT16_MIN) return INT16_MIN;
  if (i > INT16_MAX) return INT16_MAX;
  return i;
}

uint16_t
clip_ui32_to_ui16(
  uint32_t i)
{
  if (i > UINT16_MAX) return UINT16_MAX;
  return i;
}

// TODO: improve according to
// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetNaive
// https://www.chessprogramming.org/Population_Count#The_PopCount_routine
// could use __popcnt64
int
numbits(
  uint64_t i)
{
#if defined(__GNUC__) && ULONG_MAX == UINT64_MAX
  return __builtin_popcountl(i);
#elif defined(__GNUC__) && ULLONG_MAX == UINT64_MAX
  return __builtin_popcountll(i);
#else
  int n = 0;
  for (; i != 0; n++, i &= i-1);
  return n;
#endif
}

int32_t
fastround(
  double d)
{
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
  int i;
// flds for float
  asm("fldl %[d]\n\t"
      "fistpl %[i]" : [i]"=m"(i) : [d]"m"(d) : );
  return i;
#else
  int32_t i = 0;
  d += 68719476736.0 * 1.5;
  memcpy(&i, &d, 4); // If big-endian, copy from &d + 4
  return i >> 16;
//  return ((int32_t *)&d)[0] >> 16; // If big-endian, use 1 as index
#endif
}

double
normalize_angle(
  double a)
{
  double b = fmod(a, 2.0 * M_PI);    // [-2Pi; 2Pi]
  if (b < 0.0) b += 2.0 * M_PI;      // [0; 2Pi[
  else if (b == 2.0 * M_PI) b = 0.0; // [0; 2Pi[
  return b;
}
