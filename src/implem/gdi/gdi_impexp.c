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
// on real windows, this contains C++ stuff
// and the correct include is GdiplusGpStubs.h
//#include <gdiplus/gdiplustypes.h>
// on real windows SDK, this is just gdiplusflat.h
// tweak dune file, if pure windows, don't include gdiplus
// or, including gdiplus anyways does not hurt ?
//#include <gdiplus/gdiplusflat.h>
#include <gdiplus/gdiplus.h> // compiles differently if C++

#include "../unicode.h"
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

#else

const int gdi_impexp = 0;

#endif /* HAS_GDI */
