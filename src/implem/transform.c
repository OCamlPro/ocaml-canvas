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
#include <stdbool.h>
#include <math.h>
#include <assert.h>

#include "util.h"
#include "point.h"
#include "transform.h"

transform_t *
transform_create(
  void)
{
  transform_t *t = (transform_t *)calloc(1, sizeof(transform_t));
  if (t == NULL) {
    return NULL;
  }

  transform_reset(t);

  return t;
}

void
transform_destroy(
  transform_t *t)
{
  assert(t != NULL);

  free(t);
}

void
transform_reset(
  transform_t *t)
{
  assert(t != NULL);

  t->a = 1.0; t->b = 0.0;
  t->c = 0.0; t->d = 1.0;
  t->e = 0.0; t->f = 0.0;
}

transform_t *
transform_copy(
  const transform_t *t)
{
  assert(t != NULL);

  transform_t *tc = (transform_t *)calloc(1, sizeof(transform_t));
  if (tc == NULL) {
    return NULL;
  }

  tc->a = t->a; tc->b = t->b;
  tc->c = t->c; tc->d = t->d;
  tc->e = t->e; tc->f = t->f;

  return tc;
}

void
transform_set(
  transform_t *t,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f)
{
  assert(t != NULL);

  t->a = a; t->b = b;
  t->c = c; t->d = d;
  t->e = e; t->f = f;
}

/*
   a' b' 0   a b 0   a*a'+c*b'    b*a'+d*b'   0
   c' d' 0 * c d 0 = a*c'+c*d'    b*c'+d*d'   0
   e' f' 1   e f 1   e+a*e'+c*f'  f+b*e'+d*f' 1
*/
void
transform_mul(
  transform_t *t,
  double a,
  double b,
  double c,
  double d,
  double e,
  double f)
{
  assert(t != NULL);

  double aa = t->a * a + t->c * b;
  double bb = t->b * a + t->d * b;
  double cc = t->a * c + t->c * d;
  double dd = t->b * c + t->d * d;
  double ee = t->e + t->a * e + t->c * f;
  double ff = t->f + t->b * e + t->d * f;
  t->a = aa; t->b = bb;
  t->c = cc; t->d = dd;
  t->e = ee; t->f = ff;
}

/*
   1  0  0   a b 0         a           b     0
   0  1  0 * c d 0 =       c           d     0
   e' f' 1   e f 1   e+e'*a+f'*c f+e'*b+f'*d 1
*/
void
transform_translate(
  transform_t *t,
  double x, // = e
  double y) // = f
{
  assert(t != NULL);

  t->e += x * t->a + y * t->c;
  t->f += x * t->b + y * t->d;
}

/*
   a' 0  0   a b 0   a*a' b*a' 0
   0  d' 0 * c d 0 = c*d' d*d' 0
   0  0  1   e f 1    e    f   1
*/
void
transform_scale(
  transform_t *t,
  double x, // = a
  double y) // = d
{
  assert(t != NULL);

  t->a *= x; t->b *= x;
  t->c *= y; t->d *= y;
}

/*
   1  b' 0   a b 0   a+c*b' b+d*b' 0
   c' 1  0 * c d 0 = c+a*c' d+b*c' 0
   0  0  1   e f 1     e      f    1
*/
void
transform_shear(
  transform_t *t,
  double x, // = c
  double y) // = b
{
  assert(t != NULL);

  double a = t->a + t->c * y;
  double b = t->b + t->d * y;
  double c = t->c + t->a * x;
  double d = t->d + t->b * x;
  t->a = a; t->b = b;
  t->c = c; t->d = d;
}

/*
   cos(a) -sin(a) 0   a b 0   a*cos(a)-c*sin(a) b*cos(a)-d*sin(a) 0
   sin(a)  cos(a) 0 * c d 0 = c*cos(a)+a*sin(a) d*cos(a)+b*sin(a) 0
     0       0    1   e f 1           e                 f         1
*/
void
transform_rotate(
  transform_t *t,
  double angle)
{
  assert(t != NULL);

  double cos_a = cos(-angle);
  double sin_a = sin(-angle);
  double a = t->a * cos_a - t->c * sin_a;
  double b = t->b * cos_a - t->d * sin_a;
  double c = t->c * cos_a + t->a * sin_a;
  double d = t->d * cos_a + t->b * sin_a;
  t->a = a; t->b = b;
  t->c = c; t->d = d;
}

/*
   a b 0
   c d 0
   e f 1
*/
void
transform_inverse(
  transform_t *t)
{
  assert(t != NULL);

  double det = t->a * t->d - t->b * t->c;
  double invdet = 1.0 / det;
  double a = t->d * invdet;
  double b = -t->b * invdet;
  double c = -t->c * invdet;
  double d = t->a * invdet;
  double e = (t->c * t->f - t->d * t->e) * invdet;
  double f = (t->b * t->e - t->a * t->f) * invdet;
  t->a = a; t->b = b;
  t->c = c; t->d = d;
  t->e = e; t->f = f;
  // Note: M(2,2) = (a * d - b * c) / invdet = 1
}

void
transform_apply(
  const transform_t *t,
  point_t *p)
{
  assert(t != NULL);
  assert(p != NULL);

  double x = p->x * t->a + p->y * t->c + t->e;
  double y = p->x * t->b + p->y * t->d + t->f;
  p->x = x;
  p->y = y;
}

point_t
transform_apply_new(
  const transform_t *t,
  const point_t *p)
{

  assert(p != NULL);

  if (t == NULL) {
    return *p;
  }

  point_t pp = *p;

  transform_apply(t, &pp);

  return pp;
}

static const double _epsilon = 0.0001;

bool
transform_is_identity(
  const transform_t *t)
{
  return
    between(t->a - 1.0, -_epsilon, _epsilon) &&
    between(t->d - 1.0, -_epsilon, _epsilon) &&
    between(t->b, -_epsilon, _epsilon) &&
    between(t->c, -_epsilon, _epsilon) &&
    between(t->e, -_epsilon, _epsilon) &&
    between(t->f, -_epsilon, _epsilon);
}

bool
transform_is_pure_translation(
  const transform_t *t)
{
  return
    between(t->a - 1.0, -_epsilon, _epsilon) &&
    between(t->d - 1.0, -_epsilon, _epsilon) &&
    between(t->b, -_epsilon, _epsilon) &&
    between(t->c, -_epsilon, _epsilon);
}

void
transform_extract_ft(
  const transform_t *t,
  double *a,
  double *b,
  double *c,
  double *d)
{
  assert(t != NULL);
  assert(a != NULL);
  assert(b != NULL);
  assert(c != NULL);
  assert(d != NULL);

  *a = t->a; *b = t->b;
  *c = t->c; *d = t->d;
}

void
transform_extract_translation(
  const transform_t *t,
  double *e,
  double *f)
{
  assert(t != NULL);
  assert(e != NULL);
  assert(f != NULL);

  *e = t->e; *f = t->f;
}

// https://frederic-wang.fr/decomposition-of-2d-transform-matrices.html
void
transform_extract_scale(
  const transform_t *t,
  double *sx,
  double *sy)
{
  assert(t != NULL);
  assert(sx != NULL);
  assert(sy != NULL);

  double det = t->a * t->d - t->b * t->c;
  double r = sqrt(t->a * t->a + t->b * t->b);
  *sx = r;
  *sy = det / r;
}

transform_t *
transform_extract_linear(
  const transform_t *t
)
{
  assert(t != NULL);

  transform_t *tc = (transform_t *)calloc(1, sizeof(transform_t));
  if (tc == NULL) {
    return NULL;
  }

  tc->a = t->a;
  tc->b = t->b;
  tc->c = t->c;
  tc->d = t->d;
  tc->e = 0;
  tc->f = 0;

  return tc;
}

point_t
point_offset_ortho_transform(
  point_t p1,
  point_t p2,
  double o,
  const transform_t *lin,
  const transform_t *inv_linear)
{
  point_t dp = point(p2.x - p1.x, p2.y - p1.y);

  transform_apply(inv_linear, &dp);
  point_t ortho = point(-dp.y, dp.x);
  double norm = point_dist(ortho, point(0.0, 0.0));

  transform_apply(lin, &ortho);
  ortho = point((ortho.x / norm) * o, (ortho.y / norm) * o);

  return point(p1.x + ortho.x, p1.y + ortho.y);
}
