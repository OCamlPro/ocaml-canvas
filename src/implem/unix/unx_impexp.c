/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#if defined HAS_X11 || defined HAS_WAYLAND

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <memory.h>
#include <setjmp.h>
#include <assert.h>

#include <png.h>

#include "../util.h"
#include "../pixmap.h"
#include "../color.h"

bool
unx_impexp_init(
  void)
{
  return true;
}

void
unx_impexp_terminate(
  void)
{
  return;
}

bool
unx_impexp_export_png(
  const pixmap_t *pixmap,
  const char *filename)
{
  assert(pixmap != NULL);
  assert(pixmap_valid(*pixmap));
  assert(filename != NULL);

  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    return false;
  }

  png_struct *png =
    png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    fclose(fp);
    return false;
  }

  png_info *info = png_create_info_struct(png);
  if (info == NULL) {
    png_destroy_write_struct(&png, NULL);
    fclose(fp);
    return false;
  }

  png_byte **row_pointers = NULL;

  if (setjmp(png_jmpbuf(png))) {
    if (row_pointers != NULL) {
      png_free(png, row_pointers);
    }
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return false;
  }

  png_set_check_for_invalid_index(png, 0);

  png_init_io(png, fp);

  png_set_IHDR(png, info, pixmap->width, pixmap->height, 8,
               PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  png_set_bgr(png);

  png_write_info(png, info);

  row_pointers =
    (png_byte **)png_malloc(png, pixmap->height * sizeof(png_byte *));
  if (row_pointers == NULL) {
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return false;
  }

  for (int i = 0; i < pixmap->height; i++) {
    row_pointers[i] = (png_byte *)&pixmap_at(*pixmap, i, 0);
  }

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  png_free(png, row_pointers);
  png_destroy_write_struct(&png, &info);
  fclose(fp);

  return true;
}

bool
unx_impexp_import_png(
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

  png_struct *png = NULL;
  png_info *info = NULL;
  png_byte *row = NULL;
  color_t_ *data = NULL;

  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) {
    goto error;
  }

  png_byte sig[8] = { 0 };
  size_t n = fread(sig, 1, 8, fp);
  rewind(fp);
  if ((n != 8) || (png_sig_cmp(sig, 0, 8) != 0)) {
    goto error;
  }

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png == NULL) {
    goto error;
  }

  info = png_create_info_struct(png);
  if (info == NULL) {
    goto error;
  }

  if (setjmp(png_jmpbuf(png))) {
    goto error;
  }

  png_init_io(png, fp);

  png_read_info(png, info);

  png_byte color_type = png_get_color_type(png, info);
  png_byte bit_depth = png_get_bit_depth(png, info);

  if (bit_depth == 16) {
    png_set_strip_16(png);
  }

  if (png_get_valid(png, info, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png);
  }

  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
    png_set_expand_gray_1_2_4_to_8(png);
  }

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
    png_set_gray_to_rgb(png);
  }

  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png);
  }

  if (color_type == PNG_COLOR_TYPE_RGB ||
      color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
  }

  png_set_bgr(png);

  int32_t swidth = png_get_image_width(png, info);
  int32_t sheight = png_get_image_height(png, info);

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

  row = (png_byte *)png_malloc(png, swidth * COLOR_SIZE);
  if (row == NULL) {
    goto error;
  }

  if (alloc == true) {
    data = calloc(width * height, COLOR_SIZE);
    if (data == NULL) {
      goto error;
    }
  } else {
    data = pixmap->data;
  }

  int passes = png_set_interlace_handling(png);
  png_read_update_info(png, info);

  for (int p = 0; p < passes; ++p) {
    for (int32_t i = 0; i < sy; ++i) {
      png_read_row(png, row, NULL);
    }
    for (int32_t i = 0; i < height; ++i) {
      if (p > 0) {
        memcpy(row + sx * COLOR_SIZE,
               (void *)&data[(dy + i) * dwidth + dx],
               width * COLOR_SIZE);
      }
      png_read_row(png, row, NULL);
      memcpy((void *)&data[(dy + i) * dwidth + dx],
             row + sx * COLOR_SIZE,
             width * COLOR_SIZE);
    }
    for (int32_t i = sy + height; i < sheight; ++i) {
      png_read_row(png, row, NULL);
    }
  }

  png_read_end(png, NULL);

  if (alloc == true) {
    pixmap->data = data;
    pixmap->width = dwidth;
    pixmap->height = dheight;
  }

  res = true;

error:
  if ((res == false) && (alloc == true) && (data != NULL)) {
    free(data);
  }
  if (row != NULL) {
    png_free(png, row);
  }
  if (png != NULL) {
    png_destroy_read_struct(&png, (info == NULL) ? NULL : &info, NULL);
  }
  if (fp != NULL) {
    fclose(fp);
  }
  return res;
}

#else

const int unx_impexp = 0;

#endif /* HAS_X11 || HAS_WAYLAND */
