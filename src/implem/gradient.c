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
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "util.h"
#include "color.h"
#include "transform.h"
#include "object.h"
#include "gradient.h"
#include "gradient_internal.h"

IMPLEMENT_OBJECT_METHODS(gradient_t, gradient, _gradient_destroy)

gradient_t *
gradient_create_linear(
  double pos1_x,
  double pos1_y,
  double pos2_x,
  double pos2_y)
{
  gradient_t *g = gradient_alloc();
  if (g == NULL) {
    return NULL;
  }

  g->gradient_type = GRADIENT_TYPE_LINEAR;
  g->properties.linear.pos1_x = pos1_x;
  g->properties.linear.pos2_x = pos2_x;
  g->properties.linear.pos1_y = pos1_y;
  g->properties.linear.pos2_y = pos2_y;
  g->nodes = NULL;

  return g;
}

gradient_t *
gradient_create_radial(
  double center1_x,
  double center1_y,
  double rad1,
  double center2_x,
  double center2_y,
  double rad2)
{
  assert(rad1 >= 0.0);
  assert(rad2 >= 0.0);

  gradient_t *g = gradient_alloc();
  if (g == NULL) {
    return NULL;
  }

  g->gradient_type = GRADIENT_TYPE_RADIAL;
  g->properties.radial.pos1_x = center1_x;
  g->properties.radial.pos2_x = center2_x;
  g->properties.radial.pos1_y = center1_y;
  g->properties.radial.pos2_y = center2_y;
  g->properties.radial.r1 = rad1;
  g->properties.radial.r2 = rad2;
  g->nodes = NULL;

  return g;
}

gradient_t *
gradient_create_conic(
  double center_x,
  double center_y,
  double angle)
{
  gradient_t *g = gradient_alloc();
  if (g == NULL) {
    return NULL;
  }

  g->gradient_type = GRADIENT_TYPE_CONIC;
  g->properties.conic.pos_x = center_x;
  g->properties.conic.pos_y = center_y;
  g->properties.conic.angle = angle;
  g->nodes = NULL;

  return g;
}

bool
gradient_add_color_stop(
  gradient_t *gradient,
  color_t_ color,
  double pos)
{
  assert(gradient != NULL);
  assert(pos >= 0.0 && pos <= 1.0);

  if (gradient->nodes == NULL) {

    gradient->nodes = (gradient_node_t *)calloc(1,sizeof(gradient_node_t));
    if (gradient->nodes == NULL) {
      return false;
    }
    gradient->nodes->color = color;
    gradient->nodes->pos = pos;
    gradient->nodes->next = NULL;

  } else {

    gradient_node_t *run = gradient->nodes;
    gradient_node_t *new_node =
      (gradient_node_t *)calloc(1,sizeof(gradient_node_t));
    if (new_node == NULL) {
      return false;
    }
    new_node->color = color;
    new_node->pos = pos;
    if (run->pos >= pos) {
      new_node->next = run;
      gradient->nodes = new_node;
    } else {
      while (run->pos <= pos) {
        if (run->next != NULL) {
          if (run->next->pos <= pos) {
            run = run->next;
            continue;
          } else {
            new_node->next = run->next;
            run->next = new_node;
            break;
          }
        } else {
          run->next = new_node;
          break;
        }
      }
    }

  }

  return true;
}

static color_t_
_gradient_evaluate(
  const gradient_t *gradient,
  double pos)
{
  assert(gradient != NULL);

  if (gradient->nodes == NULL) {
    return color_black;
  }

  if (pos <= gradient->nodes->pos) {
    return gradient->nodes->color;
  }

  const gradient_node_t *run = gradient->nodes;
  while (run->pos < pos) {
    if (run->next == NULL) {
      return run->color;
    } else {
      if (run->next->pos < pos) {
        run = run->next;
        continue;
      } else {
        double epsilon = 0.00001;
        if (run->next->pos - run->pos < epsilon) {
          return run->color;
        }
        double interpParam = (pos - run->pos) / (run->next->pos - run->pos);
        uint8_t alpha = fastround(interpParam * 255.0);
        return alpha_blend(alpha, run->color, run->next->color);
      }
    }
  }

  // Unlikely
  return color_of_int(0);
}

color_t_
gradient_evaluate_pos(
  const gradient_t *gradient,
  double pos_x,
  double pos_y,
  const transform_t *inverse_transform)
{
  assert(gradient != NULL);

  point_t p = point(pos_x, pos_y);
  transform_apply(inverse_transform, &p);

  switch (gradient->gradient_type) {
    case GRADIENT_TYPE_LINEAR: {
      double dx =
        (gradient->properties.linear.pos2_x -
         gradient->properties.linear.pos1_x);
      double dy =
        (gradient->properties.linear.pos2_y -
         gradient->properties.linear.pos1_y);
      double t =
        (p.x - gradient->properties.linear.pos1_x) * dx +
        (p.y - gradient->properties.linear.pos1_y) * dy;
      t /= (dx * dx + dy * dy);
      return _gradient_evaluate(gradient, t);
      break;
    }
    case GRADIENT_TYPE_RADIAL: {
      double r0 = gradient->properties.radial.r1;
      double r1 = gradient->properties.radial.r2;
      double x1 =
        gradient->properties.radial.pos2_x -
        gradient->properties.radial.pos1_x;
      double x = p.x - gradient->properties.radial.pos1_x;
      double y1 =
        gradient->properties.radial.pos2_y -
        gradient->properties.radial.pos1_y;
      double y = p.y - gradient->properties.radial.pos1_y;
      double dr = r1 - r0;
      double a = (x1 * x1 + y1 * y1 - dr * dr);
      // No need for an if statement, replace a with a small number
      // since the solution is continuous with respect to a.
      if (a == 0.0) {
        a = 0.000001;
      }
      double b = (-2.0 * x * x1 - 2.0 * y * y1 - 2.0 * r0 * dr);
      double c = x * x + y * y - r0 * r0;
      double delta = b * b - 4.0 * a * c;
      if (delta < 0.0) {
        return color(0, 0, 0, 0);
      }
      double posSqrt = (a > 0.0) ? sqrt(delta) : -sqrt(delta);
      double t = (posSqrt - b) / (2.0 * a);
      // TODO : The interpolation seem to be clipped at a certain point in JS.
      // Test JS to find this number
      double infinity = 10000000.0;
      if (t > infinity) {
        return color(0, 0, 0, 0);
      }
      return _gradient_evaluate(gradient, t);
      break;
    }
    case GRADIENT_TYPE_CONIC: {
      double dx = p.x - gradient->properties.conic.pos_x;
      double dy = p.y - gradient->properties.conic.pos_y;
      double angle = atan2(dx, - dy) - gradient->properties.conic.angle;
      angle = angle / (2.0 * M_PI) -  floor(angle / (2.0 * M_PI));
      return _gradient_evaluate(gradient, angle);
      break;
    }
    default:
      return color_transparent_black;
      break;
  }
}

static void
_gradient_free_nodes(
  gradient_node_t *node)
{
  assert(node != NULL);

  if (node->next != NULL) {
    _gradient_free_nodes(node->next);
  }
  free(node);
}

static void (*_gradient_destroy_callback)(gradient_t *) = NULL;

void
gradient_set_destroy_callback(
  void (*callback)(gradient_t *))
{
  _gradient_destroy_callback = callback;
}

static void
_gradient_destroy(
  gradient_t *gradient)
{
  assert(gradient != NULL);

  if (_gradient_destroy_callback != NULL) {
    _gradient_destroy_callback(gradient);
  }
  if (gradient->nodes != NULL) {
    _gradient_free_nodes(gradient->nodes);
  }
  free(gradient);
}
