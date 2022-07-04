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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <windows.h>
#include <gdiplus/gdiplus.h>

#include "../unicode.h"
#include "../util.h"
#include "../color.h"

static bool _gdi_impexp_initialized = false;
static ULONG_PTR _gdi_impexp_gdiplus_token = 0;

bool
gdi_impexp_init(
  void)
{
  if (_gdi_impexp_initialized == false) {
    const GdiplusStartupInput startup_input = { 1, NULL, FALSE, TRUE };
    GpStatus status =
      GdiplusStartup(&_gdi_impexp_gdiplus_token, &startup_input, NULL);
    _gdi_impexp_initialized = (status == Ok);
  }
  return _gdi_impexp_initialized;
}

void
gdi_impexp_terminate(
  void)
{
  if (_gdi_impexp_initialized == true) {
    // Can it fail ? What to do if it fails ?
    GdiplusShutdown(_gdi_impexp_gdiplus_token);
    _gdi_impexp_gdiplus_token = 0;
    _gdi_impexp_initialized = false;
  }
  return;
}

static bool
_gdi_find_encoder(
  const wchar_t *mime_type,
  CLSID *clsid)
{
  assert(_gdi_impexp_initialized == true);
  assert(mime_type != NULL);
  assert(clsid != NULL);

  UINT nb_encoders, encoders_size;
  GpStatus status = GdipGetImageEncodersSize(&nb_encoders, &encoders_size);
  if (status != Ok) {
    return false;
  }

  ImageCodecInfo *codec_info = (ImageCodecInfo *)(malloc(encoders_size));
  if (codec_info == NULL) {
    return false;
  }

  status = GdipGetImageEncoders(nb_encoders, encoders_size, codec_info);
  if (status != Ok) {
    free(codec_info);
    return false;
  }

  for (UINT i = 0; i < nb_encoders; ++i) {
    if (wcscmp(codec_info[i].MimeType, mime_type) == 0) {
      *clsid = codec_info[i].Clsid;
      free(codec_info);
      return true;
    }
  }

  free(codec_info);
  return false;
}

bool
gdi_impexp_export_png(
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename)
{
  assert(_gdi_impexp_initialized == true);
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);
  assert(filename != NULL);

  CLSID clsid;
  if (_gdi_find_encoder(L"image/png", &clsid) == false) {
    return false;
  }

  WCHAR *wfilename = mbs_to_wcs(filename);
  if (wfilename == NULL) {
    return false;
  }

  GpBitmap *bitmap = NULL;
  GpStatus status =
    GdipCreateBitmapFromScan0(width, height, width * COLOR_SIZE,
                              PixelFormat32bppARGB, (BYTE *)data, &bitmap);
  if (status != Ok) {
    free(wfilename);
    return false;
  }

  status = GdipSaveImageToFile((GpImage *)bitmap, wfilename, &clsid, NULL);
  GdipDisposeImage((GpImage *)bitmap);
  free(wfilename);

  return (status == Ok);
}

static bool
_gdi_impexp_get_bitmap_size(
  GpBitmap *bitmap,
  int32_t *p_width,
  int32_t *p_height)
{
  assert(bitmap != NULL);
  assert(p_width != NULL);
  assert(p_height != NULL);

  UINT width = 0;
  UINT height = 0;

  GpStatus status = GdipGetImageWidth((GpImage *)bitmap, &width);
  if (status != Ok) {
    return false;
  }

  status = GdipGetImageHeight((GpImage *)bitmap, &height);
  if (status != Ok) {
    return false;
  }

  *p_width = (int32_t)width;
  *p_height = (int32_t)height;

  return true;
}

bool
gdi_impexp_import_png( // Actually, this imports any supported format
  color_t_ **p_data,
  int32_t *p_width,
  int32_t *p_height,
  int32_t dx,
  int32_t dy,
  const char *filename)
{
  assert(_gdi_impexp_initialized == true);
  assert(p_data != NULL);
  assert((*p_data != NULL) || ((dx == 0) && (dy == 0)));
  assert(p_width != NULL);
  assert(p_height != NULL);
  assert((*p_data == NULL) || ((*p_width > 0) && (*p_height > 0)));
  assert(filename != NULL);

  bool res = false;

  bool alloc = (*p_data == NULL);

  color_t_ *data = NULL;

  WCHAR *wfilename = mbs_to_wcs(filename);
  if (wfilename == NULL) {
    goto error;
  }

  GpBitmap *bitmap = NULL;
  GpStatus status = GdipCreateBitmapFromFile(wfilename, &bitmap);
  if (status != Ok) {
    goto error;
  }

  int32_t swidth = 0, sheight = 0;
  if (_gdi_impexp_get_bitmap_size(bitmap, &swidth, &sheight) == false) {
    goto error;
  }

  int32_t dwidth = 0, dheight = 0;
  if (alloc == true) {
    dwidth = swidth;
    dheight = sheight;
  } else {
    dwidth = *p_width;
    dheight = *p_height;
  }

  int32_t sx = 0, sy = 0;
  int32_t width = swidth, height = sheight;
  adjust_blit_info(dwidth, dheight, dx, dy,
                   swidth, sheight, sx, sy,
                   width, height);

  if (alloc == true) {
    data = calloc(width * height, COLOR_SIZE);
    if (data == NULL) {
      goto error;
    }
  } else {
    data = *p_data;
  }

  GpRect src_rect = { sx, sy, width, height };
  BitmapData bitmap_data = {
    width, height, dwidth * COLOR_SIZE, PixelFormat32bppARGB,
    (void *)data + (dy * dwidth + dx) * COLOR_SIZE, (UINT_PTR)NULL
  };
  status = GdipBitmapLockBits(bitmap, &src_rect,
                              ImageLockModeRead | ImageLockModeUserInputBuf,
                              PixelFormat32bppARGB, &bitmap_data);
  if (status != Ok) {
    goto error;
  }

  if (alloc == true) {
    *p_data = data;
    *p_width = dwidth;
    *p_height = dheight;
  }

  res = true;

error:
  if ((res == false) && (alloc == true) && (data != NULL)) {
    free(data);
  }
  if (bitmap != NULL) {
    GdipDisposeImage((GpImage *)bitmap);
  }
  if (wfilename != NULL) {
    free(wfilename);
  }
  return res;
}

#else

const int gdi_impexp = 0;

#endif /* HAS_GDI */
