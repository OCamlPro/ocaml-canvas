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
#include <assert.h>

#define CAML_NAME_SPACE
#include "caml/version.h"
#include "caml/memory.h"
#include "caml/alloc.h"
#include "caml/bigarray.h"
#include "caml/fail.h"
#include "caml/callback.h"

#include "../implem/config.h"
#include "../implem/tuples.h"
#include "../implem/color.h"
#include "../implem/event.h"
#include "../implem/canvas.h"
#include "../implem/backend.h"

#include "ml_tags.h"
#include "ml_convert.h"

#define BYTECODE_STUB_6(name) \
CAMLprim value name(value *a, int n) \
{ return name##_n(a[0],a[1],a[2],a[3],a[4],a[5]); }

#define BYTECODE_STUB_7(name) \
CAMLprim value name(value *a, int n) \
{ return name##_n(a[0],a[1],a[2],a[3],a[4],a[5],a[6]); }

#define BYTECODE_STUB_8(name) \
CAMLprim value name(value *a, int n) \
{ return name##_n(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7]); }

#define BYTECODE_STUB_9(name) \
CAMLprim value name(value *a, int n) \
{ return name##_n(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]); }

#define BYTECODE_STUB_10(name) \
CAMLprim value name(value *a, int n) \
{ return name##_n(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9]); }



/* Comparison */

static int32_t
_ml_canvas_get_id_raw(
  value mlCanvas)
{
  canvas_t *canvas = *((canvas_t **)Data_custom_val(mlCanvas));
  if (canvas == NULL) {
    return 0;
  } else {
    return canvas_get_id(canvas);
  }
}

intnat
ml_canvas_hash_raw(
  value mlCanvas)
{
  return (intnat)_ml_canvas_get_id_raw(mlCanvas);
}

CAMLprim value
ml_canvas_hash(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturn(Val_long(ml_canvas_hash_raw(mlCanvas)));
}

int
ml_canvas_compare_raw(
  value mlCanvas1,
  value mlCanvas2)
{
  int32_t id1 = _ml_canvas_get_id_raw(mlCanvas1);
  int32_t id2 = _ml_canvas_get_id_raw(mlCanvas2);
  if (id1 < id2) {
    return -1;
  }
  else if (id1 > id2) {
    return 1;
  }
  else {
    return 0;
  }
}

CAMLprim value
ml_canvas_compare(
  value mlCanvas1,
  value mlCanvas2)
{
  CAMLparam2(mlCanvas1, mlCanvas2);
  CAMLreturn(Val_int(ml_canvas_compare_raw(mlCanvas1, mlCanvas2)));
}



/* Creation / destruction */

CAMLprim value
ml_canvas_create_framed(
  value mlTitle,
  value mlPos,
  value mlSize)
{
  CAMLparam3(mlTitle, mlPos, mlSize);
  CAMLlocal1(mlCanvas);
  const char *title = String_val(mlTitle);
  mlCanvas = Val_canvas(
    canvas_create_framed(title,
                         Int32_val_clip(Field(mlPos, 0)),
                         Int32_val_clip(Field(mlPos, 1)),
                         Int32_val_clip(Field(mlSize, 0)),
                         Int32_val_clip(Field(mlSize, 1))));
  CAMLreturn(mlCanvas);
}

CAMLprim value
ml_canvas_create_frameless(
  value mlPos,
  value mlSize)
{
  CAMLparam2(mlPos, mlSize);
  CAMLlocal1(mlCanvas);
  mlCanvas = Val_canvas(
    canvas_create_frameless(Int32_val_clip(Field(mlPos, 0)),
                            Int32_val_clip(Field(mlPos, 1)),
                            Int32_val_clip(Field(mlSize, 0)),
                            Int32_val_clip(Field(mlSize, 1))));
  CAMLreturn(mlCanvas);
}

CAMLprim value
ml_canvas_create_offscreen(
  value mlSize)
{
  CAMLparam1(mlSize);
  CAMLlocal1(mlCanvas);
  mlCanvas = Val_canvas(
    canvas_create_offscreen(Int32_val_clip(Field(mlSize, 0)),
                            Int32_val_clip(Field(mlSize, 1))));
  CAMLreturn(mlCanvas);
}

CAMLprim value
ml_canvas_destroy(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_t *canvas = Canvas_val(mlCanvas);
  value *mlCanvas_ptr = canvas_get_user_data(canvas);
  if (mlCanvas_ptr != NULL) {
    caml_remove_generational_global_root(mlCanvas_ptr);
    canvas_set_user_data(canvas, NULL);
    free(mlCanvas_ptr);
  }
  canvas_destroy(canvas);
  Nullify_val(mlCanvas);
  CAMLreturn(Val_unit);
}



/* Visibility */

CAMLprim value
ml_canvas_show(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_show(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_hide(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_hide(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}



/* Configuration */

CAMLprim value
ml_canvas_get_id(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturn(Val_long(_ml_canvas_get_id_raw(mlCanvas)));
}

CAMLprim value
ml_canvas_get_size(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLlocal1(mlResult);
  pair_t(int32_t) result = canvas_get_size(Canvas_val(mlCanvas));
  mlResult = caml_alloc_tuple(2);
  Store_field(mlResult, 0, Val_int32_clip(fst(result)));
  Store_field(mlResult, 1, Val_int32_clip(snd(result)));
  CAMLreturn(mlResult);
}

CAMLprim value
ml_canvas_set_size(
  value mlCanvas,
  value mlSize)
{
  CAMLparam2(mlCanvas, mlSize);
  canvas_set_size(Canvas_val(mlCanvas),
                  Int32_val_clip(Field(mlSize, 0)),
                  Int32_val_clip(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_get_position(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLlocal1(mlResult);
  pair_t(int32_t) result = canvas_get_position(Canvas_val(mlCanvas));
  mlResult = caml_alloc_tuple(2);
  Store_field(mlResult, 0, Val_int32_clip(fst(result)));
  Store_field(mlResult, 1, Val_int32_clip(snd(result)));
  CAMLreturn(mlResult);
}

CAMLprim value
ml_canvas_set_position(
  value mlCanvas,
  value mlPos)
{
  CAMLparam2(mlCanvas, mlPos);
  canvas_set_position(Canvas_val(mlCanvas),
                      Int32_val_clip(Field(mlPos, 0)),
                      Int32_val_clip(Field(mlPos, 1)));
  CAMLreturn(Val_unit);
}



/* Transform */

CAMLprim value
ml_canvas_set_transform(
  value mlCanvas,
  value mlTrans)
{
  CAMLparam2(mlCanvas, mlTrans);
  canvas_set_transform(Canvas_val(mlCanvas),
                       Double_val(Field(mlTrans, 0)),
                       Double_val(Field(mlTrans, 1)),
                       Double_val(Field(mlTrans, 2)),
                       Double_val(Field(mlTrans, 3)),
                       Double_val(Field(mlTrans, 4)),
                       Double_val(Field(mlTrans, 5)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_transform(
  value mlCanvas,
  value mlTrans)
{
  CAMLparam2(mlCanvas, mlTrans);
  canvas_transform(Canvas_val(mlCanvas),
                   Double_val(Field(mlTrans, 0)),
                   Double_val(Field(mlTrans, 1)),
                   Double_val(Field(mlTrans, 2)),
                   Double_val(Field(mlTrans, 3)),
                   Double_val(Field(mlTrans, 4)),
                   Double_val(Field(mlTrans, 5)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_translate(
  value mlCanvas,
  value mlVec)
{
  CAMLparam2(mlCanvas, mlVec);
  canvas_translate(Canvas_val(mlCanvas),
                   Double_val(Field(mlVec, 0)),
                   Double_val(Field(mlVec, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_scale(
  value mlCanvas,
  value mlVec)
{
  CAMLparam2(mlCanvas, mlVec);
  canvas_scale(Canvas_val(mlCanvas),
               Double_val(Field(mlVec, 0)),
               Double_val(Field(mlVec, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_shear(
  value mlCanvas,
  value mlVec)
{
  CAMLparam2(mlCanvas, mlVec);
  canvas_shear(Canvas_val(mlCanvas),
               Double_val(Field(mlVec, 0)),
               Double_val(Field(mlVec, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_rotate(
  value mlCanvas,
  value mlTheta)
{
  CAMLparam2(mlCanvas, mlTheta);
  canvas_rotate(Canvas_val(mlCanvas),
                Double_val(mlTheta));
  CAMLreturn(Val_unit);
}



/* State */

CAMLprim value
ml_canvas_save(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_save(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_restore(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_restore(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}



/* Style / config */

CAMLprim value
ml_canvas_get_line_width(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturn(caml_copy_double(canvas_get_line_width(Canvas_val(mlCanvas))));
}

CAMLprim value
ml_canvas_set_line_width(
  value mlCanvas,
  value mlLineWidth)
{
  CAMLparam2(mlCanvas, mlLineWidth);
  canvas_set_line_width(Canvas_val(mlCanvas),
                        Double_val(mlLineWidth));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_get_stroke_color(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturn(Val_int(color_to_int(
             canvas_get_stroke_color(Canvas_val(mlCanvas))) & 0x00FFFFFF));
}

CAMLprim value
ml_canvas_set_stroke_color(
  value mlCanvas,
  value mlColor)
{
  CAMLparam2(mlCanvas, mlColor);
  canvas_set_stroke_color(Canvas_val(mlCanvas),
                          color_of_int(Unsigned_int_val(mlColor) | 0xFF000000));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_get_fill_color(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  CAMLreturn(Val_int(color_to_int(
             canvas_get_fill_color(Canvas_val(mlCanvas))) & 0x00FFFFFF));
}

CAMLprim value
ml_canvas_set_fill_color(
  value mlCanvas,
  value mlColor)
{
  CAMLparam2(mlCanvas, mlColor);
  canvas_set_fill_color(Canvas_val(mlCanvas),
                        color_of_int(Unsigned_int_val(mlColor) | 0xFF000000));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_set_font(
  value mlCanvas,
  value mlFamily,
  value mlSize,
  value mlSlant,
  value mlWeight)
{
  CAMLparam5(mlCanvas, mlFamily, mlSize, mlSlant, mlWeight);
  canvas_set_font(Canvas_val(mlCanvas),
                  String_val(mlFamily),
                  Double_val(mlSize),
                  Slant_val(mlSlant),
                  Int_val(mlWeight));
  CAMLreturn(Val_unit);
}



/* Paths */

CAMLprim value
ml_canvas_clear_path(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_clear_path(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_close_path(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_close_path(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_move_to(
  value mlCanvas,
  value mlP)
{
  CAMLparam2(mlCanvas, mlP);
  canvas_move_to(Canvas_val(mlCanvas),
                 Double_val(Field(mlP, 0)),
                 Double_val(Field(mlP, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_line_to(
  value mlCanvas,
  value mlP)
{
  CAMLparam2(mlCanvas, mlP);
  canvas_line_to(Canvas_val(mlCanvas),
                 Double_val(Field(mlP, 0)),
                 Double_val(Field(mlP, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_arc_n(
  value mlCanvas,
  value mlP,
  value mlRadius,
  value mlTheta1,
  value mlTheta2,
  value mlCcw)
{
  CAMLparam5(mlCanvas, mlP, mlRadius, mlTheta1, mlTheta2);
  CAMLxparam1(mlCcw);
  canvas_arc(Canvas_val(mlCanvas),
             Double_val(Field(mlP, 0)),
             Double_val(Field(mlP, 1)),
             Double_val(mlRadius),
             Double_val(mlTheta1),
             Double_val(mlTheta2),
             Bool_val(mlCcw));
  CAMLreturn(Val_unit);
}

BYTECODE_STUB_6(ml_canvas_arc)

CAMLprim value
ml_canvas_arc_to(
  value mlCanvas,
  value mlP1,
  value mlP2,
  value mlRadius)
{
  CAMLparam4(mlCanvas, mlP1, mlP2, mlRadius);
  canvas_arc_to(Canvas_val(mlCanvas),
                Double_val(Field(mlP1, 0)),
                Double_val(Field(mlP1, 1)),
                Double_val(Field(mlP2, 0)),
                Double_val(Field(mlP2, 1)),
                Double_val(mlRadius));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_quadratic_curve_to(
  value mlCanvas,
  value mlCP,
  value mlP)
{
  CAMLparam3(mlCanvas, mlCP, mlP);
  canvas_quadratic_curve_to(Canvas_val(mlCanvas),
                            Double_val(Field(mlCP, 0)),
                            Double_val(Field(mlCP, 1)),
                            Double_val(Field(mlP, 0)),
                            Double_val(Field(mlP, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_bezier_curve_to(
  value mlCanvas,
  value mlCP1,
  value mlCP2,
  value mlP)
{
  CAMLparam4(mlCanvas, mlCP1, mlCP2, mlP);
  canvas_bezier_curve_to(Canvas_val(mlCanvas),
                         Double_val(Field(mlCP1, 0)),
                         Double_val(Field(mlCP1, 1)),
                         Double_val(Field(mlCP2, 0)),
                         Double_val(Field(mlCP2, 1)),
                         Double_val(Field(mlP, 0)),
                         Double_val(Field(mlP, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_rect(
  value mlCanvas,
  value mlP,
  value mlSize)
{
  CAMLparam3(mlCanvas, mlP, mlSize);
  canvas_rect(Canvas_val(mlCanvas),
              Double_val(Field(mlP, 0)),
              Double_val(Field(mlP, 1)),
              Double_val(Field(mlSize, 0)),
              Double_val(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_ellipse_n(
  value mlCanvas,
  value mlP,
  value mlRadius,
  value mlRotation,
  value mlTheta1,
  value mlTheta2,
  value mlCcw)
{
  CAMLparam5(mlCanvas, mlP, mlRadius, mlRotation, mlTheta1);
  CAMLxparam2(mlTheta2, mlCcw);
  canvas_ellipse(Canvas_val(mlCanvas),
                 Double_val(Field(mlP, 0)),
                 Double_val(Field(mlP, 1)),
                 Double_val(Field(mlRadius, 0)),
                 Double_val(Field(mlRadius, 1)),
                 Double_val(mlRotation),
                 Double_val(mlTheta1),
                 Double_val(mlTheta2),
                 Bool_val(mlCcw));
  CAMLreturn(Val_unit);
}

BYTECODE_STUB_7(ml_canvas_ellipse)



/* Path stroking/filling */

CAMLprim value
ml_canvas_fill(
  value mlCanvas,
  value mlNonZero)
{
  CAMLparam2(mlCanvas, mlNonZero);
  canvas_fill(Canvas_val(mlCanvas),
              Bool_val(mlNonZero));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_stroke(
  value mlCanvas)
{
  CAMLparam1(mlCanvas);
  canvas_stroke(Canvas_val(mlCanvas));
  CAMLreturn(Val_unit);
}



/* Immediate drawing */

CAMLprim value
ml_canvas_fill_rect(
  value mlCanvas,
  value mlP,
  value mlSize)
{
  CAMLparam3(mlCanvas, mlP, mlSize);
  canvas_fill_rect(Canvas_val(mlCanvas),
                   Double_val(Field(mlP, 0)),
                   Double_val(Field(mlP, 1)),
                   Double_val(Field(mlSize, 0)),
                   Double_val(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_stroke_rect(
  value mlCanvas,
  value mlP,
  value mlSize)
{
  CAMLparam3(mlCanvas, mlP, mlSize);
  canvas_stroke_rect(Canvas_val(mlCanvas),
                     Double_val(Field(mlP, 0)),
                     Double_val(Field(mlP, 1)),
                     Double_val(Field(mlSize, 0)),
                     Double_val(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_fill_text(
  value mlCanvas,
  value mlText,
  value mlP)
{
  CAMLparam3(mlCanvas, mlText, mlP);
  canvas_fill_text(Canvas_val(mlCanvas),
                   String_val(mlText),
                   Double_val(Field(mlP, 0)),
                   Double_val(Field(mlP, 1)),
                   0.0);
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_stroke_text(
  value mlCanvas,
  value mlText,
  value mlP)
{
  CAMLparam3(mlCanvas, mlText, mlP);
  canvas_stroke_text(Canvas_val(mlCanvas),
                     String_val(mlText),
                     Double_val(Field(mlP, 0)),
                     Double_val(Field(mlP, 1)),
                     0.0);
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_blit(
  value mlDstCanvas,
  value mlDPos,
  value mlSrcCanvas,
  value mlSPos,
  value mlSize)
{
  CAMLparam5(mlDstCanvas, mlDPos, mlSrcCanvas, mlSPos, mlSize);
  canvas_blit(Canvas_val(mlDstCanvas),
              Int32_val_clip(Field(mlDPos, 0)),
              Int32_val_clip(Field(mlDPos, 1)),
              Canvas_val(mlSrcCanvas),
              Int32_val_clip(Field(mlSPos, 0)),
              Int32_val_clip(Field(mlSPos, 1)),
              Int32_val_clip(Field(mlSize, 0)),
              Int32_val_clip(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}



/* Direct pixel access */

CAMLprim value
ml_canvas_get_pixel(
  value mlCanvas,
  value mlPos)
{
  CAMLparam2(mlCanvas, mlPos);
  CAMLreturn(Val_int(color_to_int(
             canvas_get_pixel(Canvas_val(mlCanvas),
             Int32_val_clip(Field(mlPos, 0)),
             Int32_val_clip(Field(mlPos, 1)))) & 0x00FFFFFF));
}

CAMLprim value
ml_canvas_set_pixel(
  value mlCanvas,
  value mlPos,
  value mlColor)
{
  CAMLparam3(mlCanvas, mlPos, mlColor);
  canvas_set_pixel(Canvas_val(mlCanvas),
                   Int32_val_clip(Field(mlPos, 0)),
                   Int32_val_clip(Field(mlPos, 1)),
                   color_of_int(Unsigned_int_val(mlColor) | 0xFF000000));
  CAMLreturn(Val_unit);
}



intnat dim[CAML_BA_MAX_NUM_DIMS] = { 5, 5, 4 };

CAMLprim value
ml_canvas_get_image_data(
  value mlCanvas,
  value mlPos,
  value mlSize)
{
  CAMLparam3(mlCanvas, mlPos, mlSize);
  image_data_t image_data =
    canvas_get_image_data(Canvas_val(mlCanvas),
                          Int32_val_clip(Field(mlPos, 0)),
                          Int32_val_clip(Field(mlPos, 1)),
                          Int32_val_clip(Field(mlSize, 0)),
                          Int32_val_clip(Field(mlSize, 1)));
  if (image_data_valid(image_data) == false) {
    printf("Failed\n");
  }
  CAMLreturn(Val_image_data(&image_data));
}

CAMLprim value
ml_canvas_set_image_data(
  value mlCanvas,
  value mlDPos,
  value mlImageData,
  value mlSPos,
  value mlSize)
{
  CAMLparam5(mlCanvas, mlDPos, mlImageData, mlSPos, mlSize);
  image_data_t image_data = Image_data_val(mlImageData);
  canvas_set_image_data(Canvas_val(mlCanvas),
                        Int32_val_clip(Field(mlDPos, 0)),
                        Int32_val_clip(Field(mlDPos, 1)),
                        &image_data,
                        Int32_val_clip(Field(mlSPos, 0)),
                        Int32_val_clip(Field(mlSPos, 1)),
                        Int32_val_clip(Field(mlSize, 0)),
                        Int32_val_clip(Field(mlSize, 1)));
  CAMLreturn(Val_unit);
}



/* Event */

CAMLprim value
ml_canvas_int_of_key(
  value mlKeyCode)
{
  CAMLparam1(mlKeyCode);
  CAMLreturn(Val_int(Key_code_val(mlKeyCode)));
}

CAMLprim value
ml_canvas_key_of_int(
  value mlInt)
{
  CAMLparam1(mlInt);
  CAMLreturn(Val_key_code(Int_val(mlInt)));
}



/* Backend */

static bool _ml_canvas_initialized = false;

static impl_type_t
_ml_canvas_impl_type(
  value mlBackend)
{
  CAMLparam1(mlBackend);
  impl_type_t impl_type = IMPL_NONE;
  switch (Int_val(mlBackend)) {
    case TAG_CANVAS:  assert(!"HTML5 Canvas not available from C"); break;
    case TAG_GDI:     impl_type = IMPL_GDI;     break;
    case TAG_QUARTZ:  impl_type = IMPL_QUARTZ;  break;
    case TAG_X11:     impl_type = IMPL_X11;     break;
    case TAG_WAYLAND: impl_type = IMPL_WAYLAND; break;
    default:          assert(!"Invalid backend specified"); break;
  }
  CAMLreturnT(impl_type_t, impl_type);
}

CAMLprim value
ml_canvas_init(
  value mlOptions)
{
  CAMLparam1(mlOptions);
  CAMLlocal2(mlBackend, mlBackendList);

  if (_ml_canvas_initialized) {
    CAMLreturn(Val_false);
  }

  switch (get_os_type()) {
    case OS_WIN32: mlBackendList = Field(mlOptions, 1); break;
    case OS_OSX: mlBackendList = Field(mlOptions, 2); break;
    case OS_UNIX: mlBackendList = Field(mlOptions, 3); break;
    default: mlBackendList = Field(mlOptions, 3); break;
  }

  while ((mlBackendList != Val_emptylist) && (!_ml_canvas_initialized)) {
    mlBackend = Field(mlBackendList, 0);
    mlBackendList = Field(mlBackendList, 1);
    impl_type_t impl_type = _ml_canvas_impl_type(mlBackend);
    _ml_canvas_initialized = backend_init(impl_type);
  }

  CAMLreturn(Val_bool(_ml_canvas_initialized));
}

static value _ml_canvas_mlException = Val_unit;
static value _ml_canvas_mlProcessEvent = Val_unit;

static bool
_ml_canvas_process_event(
  event_t *event,
  event_listener_t *next_listener)
{
  CAMLparam0();
  CAMLlocal1(mlResult);

  if (_ml_canvas_mlProcessEvent == Val_unit) {
    CAMLreturnT(bool, false);
  }

  mlResult = caml_callback_exn(_ml_canvas_mlProcessEvent, Val_event(event));

  /* If an exception was raised, save for later */
  if (Is_exception_result(mlResult)) {
    mlResult = Extract_exception(mlResult);
    /* Only remember the first exception */
    if (_ml_canvas_mlException == Val_unit) {
      caml_modify_generational_global_root(&_ml_canvas_mlException, mlResult);
    }
    backend_stop();
    mlResult = Val_false;
  }

  /* Special handling of close event */
  if (event->type == EVENT_CLOSE) {
    canvas_t *canvas = (canvas_t *)event->target;
    value *mlCanvas_ptr = canvas_get_user_data(canvas);
    if (mlCanvas_ptr != NULL) {
      canvas_set_user_data(canvas, NULL);
      caml_remove_generational_global_root(mlCanvas_ptr);
      Nullify_val(*mlCanvas_ptr);
      free(mlCanvas_ptr);
    }
  }

  CAMLreturnT(bool, Bool_val(mlResult));
}

CAMLprim value
ml_canvas_run(
  value mlProcessEvent,
  value mlContinuation)
{
  CAMLparam2(mlProcessEvent,mlContinuation);
  CAMLlocal1(mlResult);

  /* If already running, ignore */
  if (_ml_canvas_mlProcessEvent != Val_unit) {
    CAMLreturn(Val_unit);
  }

  caml_register_generational_global_root(&_ml_canvas_mlProcessEvent);
  caml_modify_generational_global_root(&_ml_canvas_mlProcessEvent,
                                       mlProcessEvent);

  caml_register_generational_global_root(&_ml_canvas_mlException);
  caml_modify_generational_global_root(&_ml_canvas_mlException, Val_unit);

  event_listener_t event_listener = {
    .process_event = _ml_canvas_process_event,
    .next_listener = NULL
  };
  backend_run(&event_listener);

  mlResult = _ml_canvas_mlException;
  caml_modify_generational_global_root(&_ml_canvas_mlException, Val_unit);
  caml_remove_global_root(&_ml_canvas_mlException);

  caml_modify_generational_global_root(&_ml_canvas_mlProcessEvent, Val_unit);
  caml_remove_global_root(&_ml_canvas_mlProcessEvent);

  /* If an exception was raised, re-raise */
  if (mlResult != Val_unit) {
    caml_raise(mlResult);
  }

  mlResult = caml_callback_exn(mlContinuation, Val_unit);
  if (Is_exception_result(mlResult)) {
    mlResult = Extract_exception(mlResult);
    caml_raise(mlResult);
  }

  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_stop(
  void)
{
  CAMLparam0();
  backend_stop();
  CAMLreturn(Val_unit);
}

CAMLprim value
ml_canvas_get_canvas(
  value mlId)
{
  CAMLparam1(mlId);
  canvas_t *result = backend_get_canvas(Int_val(mlId));
  if (result == NULL) {
    CAMLreturn(Val_none);
  } else {
    CAMLreturn(caml_alloc_some(Val_canvas(result)));
  }
}
