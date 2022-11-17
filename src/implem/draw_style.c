/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#include "gradient.h"
#include "pattern.h"
#include "draw_style.h"

void
draw_style_destroy(
  draw_style_t *s)
{
  assert(s != NULL);

  switch (s->type) {
    case DRAW_STYLE_COLOR:
      break;
    case DRAW_STYLE_GRADIENT:
      gradient_release(s->content.gradient);
      break;
    case DRAW_STYLE_PATTERN:
      pattern_release(s->content.pattern);
      break;
    default:
      assert(!"Invalid draw style");
      break;
  }
}

draw_style_t
draw_style_copy(
  const draw_style_t *s)
{
  assert(s != NULL);

  draw_style_t sc = *(draw_style_t *)s;
  switch (s->type) {
    case DRAW_STYLE_COLOR:
      break;
    case DRAW_STYLE_GRADIENT:
      gradient_retain(sc.content.gradient);
      break;
    case DRAW_STYLE_PATTERN:
      pattern_retain(sc.content.pattern);
      break;
    default:
      assert(!"Invalid draw style");
      break;
  }
  return sc;
}
