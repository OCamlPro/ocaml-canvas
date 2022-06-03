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
#include "gdi_backend.h"
#include "gdi_window.h"
#include "gdi_target.h"

typedef struct surface_impl_gdi_t {
  impl_type_t type;
  HBITMAP bmp;
  HDC hdc;
  HWND hwnd;
} surface_impl_gdi_t;

static HBITMAP
_surface_create_gdi_bitmap(
  HDC hdc,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(hdc != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFO); // or BITMAPINFOHEADER ?
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

surface_impl_gdi_t *
surface_create_gdi_impl(
  gdi_target_t *target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(target != NULL);
  assert(target->hwnd != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  surface_impl_gdi_t *impl =
    (surface_impl_gdi_t *)calloc(1, sizeof(surface_impl_gdi_t));
  if (impl == NULL) {
    return NULL;
  }

  HDC hdc_wnd = GetDC(target->hwnd);
  if (hdc_wnd == NULL) {
    free(impl);
    return NULL;
  }

  HDC hdc = CreateCompatibleDC(hdc_wnd);
  ReleaseDC(target->hwnd, hdc_wnd);
  if (hdc == NULL) {
    free(impl);
    return NULL;
  }

  HBITMAP bmp = _surface_create_gdi_bitmap(hdc, width, height, data);
  if (bmp == NULL) {
    assert(*data == NULL);
    DeleteDC(hdc);
    free(impl);
    return NULL;
  }

  impl->type = IMPL_GDI;
  impl->bmp = bmp;
  impl->hwnd = target->hwnd;
  impl->hdc = hdc;

  return impl;
}

void
surface_destroy_gdi_impl(
  surface_impl_gdi_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_GDI);

  if (impl->bmp) {
    DeleteDC(impl->hdc);
    DeleteObject(impl->bmp);
  }
}

static void
_raw_surface_copy(
  color_t_ *s_data,
  int32_t s_width,
  int32_t s_height,
  color_t_ *d_data,
  int32_t d_width,
  int32_t d_height)
{
  assert(s_data != NULL);
  assert(s_width > 0);
  assert(s_height > 0);
  assert(d_data != NULL);
  assert(d_width > 0);
  assert(d_height > 0);
  uint32_t min_width = d_width < s_width ? d_width : s_width;
  uint32_t min_height = d_height < s_height ? d_height : s_height;
  for (size_t i = 0; i < min_height; ++i) {
    for (size_t j = 0; j < min_width; ++j) {
      d_data[i * d_width + j] = s_data[i * s_width + j];
    }
  }
}

bool
surface_resize_gdi_impl(
  surface_impl_gdi_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  assert(impl != NULL);
  assert(s_width > 0);
  assert(s_height  > 0);
  assert(s_data != NULL);
  assert(*s_data != NULL);
  assert(d_width > 0);
  assert(d_height  > 0);
  assert(d_data != NULL);
  assert(*d_data == NULL);

  HBITMAP bmp =
    _surface_create_gdi_bitmap(impl->hdc, d_width, d_height, d_data);
  if (bmp == NULL) {
    return false;
  }

  _raw_surface_copy(*s_data, s_width, s_height, *d_data, d_width, d_height);
  //_surface_copy_to_buffer(s, data, width, height);

  if (impl->bmp) {
    DeleteObject(impl->bmp);
  }

  impl->bmp = bmp;

  return true;
}

void
surface_present_gdi_impl(
  surface_impl_gdi_t *impl,
  int32_t width,
  int32_t height,
  gdi_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height  > 0);

  //FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+2));
  //bool use_begin = (extra != NULL) && (*(bool *)extra == true);
  PAINTSTRUCT ps;
  HDC hdc =
    present_data->use_begin ? BeginPaint(impl->hwnd, &ps) : GetDC(impl->hwnd);
  BitBlt(hdc, 0, 0, width, height, impl->hdc, 0, 0, SRCCOPY);
  if (present_data->use_begin) {
    EndPaint(impl->hwnd, &ps);
  } else {
    ReleaseDC(impl->hwnd, hdc);
  }
  GdiFlush();
  ValidateRect(impl->hwnd, NULL);
}

#else

const int gdi_surface = 0;

#endif /* HAS_GDI */
