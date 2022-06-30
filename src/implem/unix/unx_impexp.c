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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <assert.h>

#include <png.h>

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
  const color_t_ *data,
  int32_t width,
  int32_t height,
  const char *filename)
{
  assert(data != NULL);
  assert(width > 0);
  assert(height > 0);
  assert(filename != NULL);

  png_byte **row_pointers = NULL;

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

  if (setjmp(png_jmpbuf(png))) {
    if (row_pointers != NULL) {
      png_free(png, row_pointers);
    };
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return false;
  }

  // Turn of a check
  png_set_check_for_invalid_index(png, 0);

/* Initialize the default input/output functions for the PNG file to standard C streams. To replace the default read and write functions, use png_set_read_fn() and png_set_write_fn() respectively.
 Again, if you wish to handle writing data in
another way, see the discussion on libpng I/O handling in the Customizing
Libpng section below. */
  png_init_io(png, fp);

// here, can set up a row callback if needed

/*
Can set filters if needed
png_set_filter(png_ptr, 0,
       PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE |
       PNG_FILTER_SUB   | PNG_FILTER_VALUE_SUB  |
       PNG_FILTER_UP    | PNG_FILTER_VALUE_UP   |
       PNG_FILTER_AVG   | PNG_FILTER_VALUE_AVG  |
       PNG_FILTER_PAETH | PNG_FILTER_VALUE_PAETH|
       PNG_ALL_FILTERS  | PNG_FAST_FILTERS);
*/

// here, can set up zlib compression level

  png_set_IHDR(png, info, width, height, 8, PNG_COLOR_TYPE_RGBA,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

// here, may call other png_set...


// High-level interface

  row_pointers = (png_byte **)png_malloc(png, height * sizeof(png_byte *));
  if (row_pointers == NULL) {
    png_destroy_write_struct(&png, &info);
    fclose(fp);
    return false;
  }

  for (int i = 0; i < height; i++) {
    row_pointers[i] = (png_byte *)&data[i * width];
  }

  png_set_rows(png, info, row_pointers); // doc says &row_pointers
                                        // does the lib try to free them ?

  png_write_png(png, info, PNG_TRANSFORM_BGR, NULL);

  png_free(png, row_pointers);

  png_destroy_write_struct(&png, &info);

  fclose(fp);

  return true;
// Low-level interface
/*
  png_write_info(png, info);

  // To remove the alpha channel for PNG_COLOR_TYPE_RGB format,
  // Use png_set_filler().
  //png_set_filler(png, 0, PNG_FILLER_AFTER);

  if (!row_pointers) abort();

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  for(int y = 0; y < height; y++) {
    free(row_pointers[y]);
  }
  free(row_pointers);
*/
}

#else

const int unx_impexp = 0;

#endif /* HAS_X11 || HAS_WAYLAND */
