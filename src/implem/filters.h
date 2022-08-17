/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifndef __FILTERS_H
#define __FILTERS_H

#include "pixmap.h"

pixmap_t
filter_gaussian_blur_alpha(
  pixmap_t *src,
  double s);

#endif /* __FILTERS_H */
