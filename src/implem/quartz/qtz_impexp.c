/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_QUARTZ

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <Cocoa/Cocoa.h>

#include "../color.h"

bool
qtz_impexp_init(
  void)
{
  return true;
}

void
qtz_impexp_terminate(
  void)
{
  return;
}

bool
qtz_impexp_export_png(
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename)
{
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);
  assert(filename != NULL);

  bool res = false;

  CFStringRef str_filename =
    CFStringCreateWithCString(kCFAllocatorDefault, filename,
                              kCFStringEncodingUTF8);
  if (str_filename == NULL) {
    goto error_string;
  }

  CFURLRef url =
    CFURLCreateWithFileSystemPath(kCFAllocatorDefault, str_filename,
                                  kCFURLPOSIXPathStyle, false);
  if (url == NULL) {
    goto error_url;
  }

  CGImageDestinationRef image_dest =
    CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
  if (image_dest == NULL) {
    goto error_image_dest;
  }

  CGColorSpaceRef color_space =
    CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
  if (color_space == NULL) {
    goto error_color_space;
  }

  CGDataProviderRef provider =
    CGDataProviderCreateWithData(NULL, (void *)data,
                                 width * height * COLOR_SIZE, NULL);
  if (provider == NULL) {
    goto error_provider;
  }

  CGImageRef image =
    CGImageCreate(width, height, 8, 8 * COLOR_SIZE,
                  width * COLOR_SIZE, color_space,
                  kCGImageAlphaPremultipliedFirst |
                  kCGBitmapByteOrder32Little,
                  provider, NULL, false,
                  kCGRenderingIntentDefault);
  if (image == NULL) {
    goto error_image;
  }

  CGImageDestinationAddImage(image_dest, image, NULL);
  if (CGImageDestinationFinalize(image_dest) == false) {
    goto error_add_image;
  }

  res = true;

error_add_image:
  CGImageRelease(image);
error_image:
  CGDataProviderRelease(provider);
error_provider:
  CGColorSpaceRelease(color_space);
error_color_space:
  CFRelease(image_dest);
error_image_dest:
  CFRelease(url);
error_url:
  CFRelease(str_filename);
error_string:
  return res;
}

#else

const int qtz_impexp = 0;

#endif /* HAS_QUARTZ */
