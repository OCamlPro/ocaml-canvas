/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_GDI

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "../config.h"
#include "../color.h"
#include "../context_internal.h"
#include "../sw_context_internal.h"
#include "gdi_target.h"

typedef struct gdi_sw_context_t {
  sw_context_t base;
  HBITMAP bmp;
  HDC hdc;
  HWND hwnd;
} gdi_sw_context_t;

static HBITMAP
_gdi_sw_context_create_bitmap(
  HDC hdc,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(hdc != NULL);
  assert(width > 0);
  assert(height > 0);
  assert(data != NULL);
  assert(*data == NULL);

  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
  bmi.bmiHeader.biWidth = width;
  bmi.bmiHeader.biHeight = -height; // must be negative, otherwise mirrored
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biSizeImage = 0;
  bmi.bmiHeader.biXPelsPerMeter = 0;
  bmi.bmiHeader.biYPelsPerMeter = 0;
  bmi.bmiHeader.biClrUsed = 0;
  bmi.bmiHeader.biClrImportant = 0;
  bmi.bmiColors[0].rgbBlue = 0;
  bmi.bmiColors[0].rgbGreen = 0;
  bmi.bmiColors[0].rgbRed = 0;
  bmi.bmiColors[0].rgbReserved = 0;

  HBITMAP bmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS,
                                 (void **)data, NULL, 0);
  GdiFlush();
  if (bmp == NULL) {
    return NULL;
  }

  SelectObject(hdc, bmp);

  return bmp;
}

gdi_sw_context_t *
gdi_sw_context_create(
  gdi_target_t *target,
  int32_t width,
  int32_t height)
{
  assert(target != NULL);
  assert(target->hwnd != NULL);
  assert(width > 0);
  assert(height > 0);

  gdi_sw_context_t *context =
    (gdi_sw_context_t *)calloc(1, sizeof(gdi_sw_context_t));
  if (context == NULL) {
    return NULL;
  }

  HDC hdc_wnd = GetDC(target->hwnd);
  if (hdc_wnd == NULL) {
    free(context);
    return NULL;
  }

  HDC hdc = CreateCompatibleDC(hdc_wnd);
  ReleaseDC(target->hwnd, hdc_wnd);
  if (hdc == NULL) {
    free(context);
    return NULL;
  }

  color_t_ *data = NULL;
  HBITMAP bmp = _gdi_sw_context_create_bitmap(hdc, width, height, &data);
  if (bmp == NULL) {
    assert(data == NULL);
    DeleteDC(hdc);
    free(context);
    return NULL;
  }
  assert(data != NULL);

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = data;
  context->bmp = bmp;
  context->hdc = hdc;
  context->hwnd = target->hwnd;

  return context;
}

void
gdi_sw_context_destroy(
  gdi_sw_context_t *context)
{
  assert(context != NULL);

  if (context->bmp != NULL) {
    DeleteObject(context->bmp); /* This also frees context->base.data */
  }

  if (context->hdc != NULL) {
    DeleteDC(context->hdc);
  }

  free(context);
}

bool
gdi_sw_context_resize(
  gdi_sw_context_t *context,
  int32_t width,
  int32_t height)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->base.data != NULL);
  assert(context->bmp != NULL);
  assert(context->hdc != NULL);
  assert(width > 0);
  assert(height > 0);

  color_t_ *data = NULL;
  HBITMAP bmp =
    _gdi_sw_context_create_bitmap(context->hdc, width, height, &data);
  if (bmp == NULL) {
    assert(data == NULL);
    return false;
  }
  assert(data != NULL);

  _sw_context_copy_to_buffer(&context->base, data, width, height);

  DeleteObject(context->bmp);

  context->base.base.width = width;
  context->base.base.height = height;
  context->base.data = data;
  context->bmp = bmp;

  return true;
}

void
gdi_sw_context_present(
  gdi_sw_context_t *context)
{
  assert(context != NULL);
  assert(context->base.base.width > 0);
  assert(context->base.base.height > 0);
  assert(context->bmp != NULL);
  assert(context->hdc != NULL);
  assert(context->hwnd != NULL);

  HDC hdc = GetDC(context->hwnd);
  BitBlt(hdc, 0, 0,
         context->base.base.width,
         context->base.base.height,
         context->hdc, 0, 0, SRCCOPY);
  ReleaseDC(context->hwnd, hdc);
  GdiFlush();
  ValidateRect(context->hwnd, NULL);
}

#else

const int gdi_sw_context = 0;

#endif /* HAS_GDI */
