/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __IMAGE_INTERPOLATION_H
#define __IMAGE_INTERPOLATION_H

#include "color.h"
#include "pixmap.h"

color_t_
interpolation_bilinear(
  const pixmap_t *image,
  double uvx,
  double uvy);

color_t_
interpolation_cubic(
  const pixmap_t *image,
  double uvx,
  double uvy);

#endif /* __IMAGE_INTERPOLATION_H */
