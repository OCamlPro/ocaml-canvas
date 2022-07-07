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

#include "../util.h"
#include "../color.h"
#include "../pixmap.h"

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
  const pixmap_t *pixmap,
  const char *filename)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap));
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
    CGDataProviderCreateWithData(NULL, (void *)pixmap->data,
                                 pixmap->width * pixmap->height * COLOR_SIZE,
                                 NULL);
  if (provider == NULL) {
    goto error_provider;
  }

  CGImageRef image =
    CGImageCreate(pixmap->width, pixmap->height, 8, 8 * COLOR_SIZE,
                  pixmap->width * COLOR_SIZE, color_space,
                  kCGImageAlphaPremultipliedFirst |
                  kCGBitmapByteOrder32Little,
                  provider, NULL, false,
                  kCGRenderingIntentDefault);
  if (image == NULL) {
    goto error_image;
  }

  CGImageDestinationAddImage(image_dest, image, NULL);
  if (CGImageDestinationFinalize(image_dest) == false) {
    goto error_finalize;
  }

  res = true;

error_finalize:
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

bool
qtz_impexp_import_png(
  pixmap_t *pixmap,
  int32_t dx,
  int32_t dy,
  const char *filename)
{
  assert(pixmap != NULL);
  assert((pixmap->data != NULL) ||
         ((dx == 0) && (dy == 0)));
  assert((pixmap->data == NULL) ||
         ((pixmap->width > 0) && (pixmap->height > 0)));
  assert(filename != NULL);

  bool res = false;

  bool alloc = (pixmap->data == NULL);

  CGDataProviderRef provider =
    CGDataProviderCreateWithFilename(filename);
  if (provider == NULL) {
    goto error_provider;
  }

  CGImageRef image =
    CGImageCreateWithPNGDataProvider(provider, NULL, false,
                                     kCGRenderingIntentDefault);
  if (image == NULL) {
    goto error_image;
  }

  CGColorSpaceRef color_space =
    CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
  if (color_space == NULL) {
    goto error_color_space;
  }

  int32_t swidth = CGImageGetWidth(image);
  int32_t sheight = CGImageGetHeight(image);

  int32_t dwidth = 0, dheight = 0;
  if (alloc == true) {
    dwidth = swidth;
    dheight = sheight;
  } else {
    dwidth = pixmap->width;
    dheight = pixmap->height;
  }

  int32_t sx = 0, sy = 0;
  int32_t width = swidth, height = sheight;
  adjust_blit_info(dwidth, dheight, dx, dy,
                   swidth, sheight, sx, sy,
                   width, height);

  color_t_ *data = NULL;
  if (alloc == true) {
    data = calloc(width * height, COLOR_SIZE);
    if (data == NULL) {
      goto error_data;
    }
  } else {
    data = pixmap->data;
  }

  CGContextRef ctxt =
    CGBitmapContextCreate((void *)data + (dy * dwidth + dx) * COLOR_SIZE,
                          width, height, 8, dwidth * COLOR_SIZE, color_space,
                          kCGImageAlphaPremultipliedFirst |
                          kCGBitmapByteOrder32Little);
  if (ctxt == NULL) {
    goto error_context;
  }

  CGContextDrawImage(ctxt, CGRectMake(-sx, height - sheight + sy,
                                      swidth, sheight), image);

  if (alloc == true) {
    pixmap->data = data;
    pixmap->width = dwidth;
    pixmap->height = dheight;
  }

  res = true;

  CGContextRelease(ctxt);
error_context:
  if ((res == false) && (alloc == true)) {
    free(data);
  }
error_data:
  CGColorSpaceRelease(color_space);
error_color_space:
  CGImageRelease(image);
error_image:
  CGDataProviderRelease(provider);
error_provider:
  return res;
}

/*
https://stackoverflow.com/questions/10412206/how-to-get-the-raw-pixel-data-from-cgimage

create a data consumer (that writes to buffer)
link with data provider (created from image)
*/

#else

const int qtz_impexp = 0;

#endif /* HAS_QUARTZ */
