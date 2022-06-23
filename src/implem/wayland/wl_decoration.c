/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 OCamlPro                                             */
/*                                                                        */
/*  All rights reserved. This file is distributed under the terms of the  */
/*  GNU Lesser General Public License version 2.1, with the special       */
/*  exception on linking described in the file LICENSE.                   */
/*                                                                        */
/**************************************************************************/

#ifdef HAS_WAYLAND

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <wayland-client.h>
#include <sys/mman.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "wl_decoration.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"
#include "wl_memory.h"
#include "../unicode.h"



void
_wl_decoration_button(
    wl_decoration_t *decor,
    struct wl_surface **surface,
    struct wl_subsurface **subsurface,
    struct wl_buffer **buffer,
    uint32_t size,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
  *surface = wl_compositor_create_surface(wl_back->compositor);
  *subsurface = wl_subcompositor_get_subsurface(wl_back->subcompositor,
                                                                    *surface,
                                                                    decor->wl_surface);
  uint8_t *data = NULL;
  *buffer = wl_create_buffer(size,size,&data);
  for (int i = 0; i < size*size;i++)
  {
    data[4*i] = b;
    data[4*i + 1] = g;
    data[4*i + 2] = r;
    data[4*i + 3] = 255;
  }
  munmap(data,size * size * 4);
  wl_surface_attach(*surface,*buffer,0,0);
}

void
_wl_decoration_render_title(
    uint8_t *target,
    uint32_t width,
    uint32_t height,
    const char* title
)
{
    assert(target != NULL);
    assert(title != NULL);

    FT_Library library;
    FT_Face face;

    uint32_t error = FT_Init_FreeType(&library);
    if (error)
        return;
    error = FT_New_Face(library,"/home/hachem/Minchou.otf",0,&face);
    if (error)
        return;
    error = FT_Set_Pixel_Sizes(face,0,16);
    uint32_t i = 0;
    uint32_t pen_x = 16;
    uint32_t pen_y = 25;
    while(*title)
    {
        uint32_t chr = decode_utf8_char(&title);
        if (pen_x > width - 30 - 16 - 70)
            break;
        if (pen_x > width - 30 - 32 - 70)
            chr = 0x0000002e;
        FT_UInt glyph_index;
        glyph_index = FT_Get_Char_Index(face,chr);
        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue;
        error = FT_Render_Glyph(face->glyph,FT_RENDER_MODE_NORMAL);
        if (error)
            continue;
        printf("%d\n",face->glyph->bitmap.rows);
        for (int r = 0; r < face->glyph->bitmap.rows;r++)
        {
            int di = pen_y - face->glyph->bitmap_top + r;
            if (di < 0) 
                continue;
            else if (di > height)
                break;
            for (int c = 0; c < face->glyph->bitmap.width;c++)
            {   
                int dj = pen_x + face->glyph->bitmap_left + c;
                if (dj < 0)
                    continue;
                if (dj > width)
                    break;
                int s_idx = r * face->glyph->bitmap.pitch + c;
                uint8_t alpha = face->glyph->bitmap.buffer[s_idx];
                int d_idx = dj + width*di;
                if (alpha == 255)
                {
                    target[4*d_idx] = 255;
                    target[4*d_idx + 1] = 255;
                    target[4*d_idx + 2] = 255;
                    target[4*d_idx + 3] = 255;
                }
                else if (alpha > 0)
                {
                  target[4*d_idx] = (alpha * 255 + (255 - alpha)*(target[4*d_idx]))/255;
                  target[4*d_idx + 1]  = (alpha * 255 + (255 - alpha)*(target[4*d_idx]))/255;
                  target[4*d_idx + 2] = (alpha * 255 + (255 - alpha)*(target[4*d_idx]))/255;
                  target[4*d_idx + 3] = 255;
                }
            }
        }
        pen_x += face->glyph->advance.x >> 6;
        pen_y += face->glyph->advance.y >> 6;
    }
    
    FT_Done_Face(face);
    FT_Done_FreeType(library);

}

struct wl_buffer*
_wl_decoration_background(
    uint32_t width,
    const char* title
)
{
    uint32_t _decor_height = 40;
    uint8_t *pool_data = NULL;
    struct wl_buffer *wl_buffer = wl_create_buffer(width,_decor_height,&pool_data);
    for (int i = 0;i < width*_decor_height;i++)
    {
        pool_data[4*i] = 0;
        pool_data[4*i + 1] = 0;
        pool_data[4*i + 2] = 0;
        pool_data[4*i + 3] = 255;
    }
    _wl_decoration_render_title(pool_data,width,_decor_height,title);
    munmap(pool_data,width * _decor_height * 4);
    return wl_buffer;
}

wl_decoration_t*
wl_decoration_create(
    struct wl_surface *parent,
    uint32_t width,
    const char* title
)
{
    //TODO : Make this adapt to screen
    uint32_t _decor_height = 40;

    assert(parent != NULL);
    wl_decoration_t *decor = (wl_decoration_t*)calloc(1, sizeof(wl_decoration_t));
    if (decor == NULL)
        return NULL;
    decor->wl_surface = wl_compositor_create_surface(wl_back->compositor);
    decor->wl_subsurface = wl_subcompositor_get_subsurface(wl_back->subcompositor,decor->wl_surface,parent);
    struct wl_buffer *wl_buffer = _wl_decoration_background(width,title);
    wl_surface_attach(decor->wl_surface,wl_buffer,0,0);
    wl_subsurface_set_position(decor->wl_subsurface,0,-_decor_height);
    decor->background_buffer = wl_buffer;
    _wl_decoration_button(decor,&decor->wl_closebutton_surface,&decor->wl_closebutton_subsurface,&decor->wl_closebutton_buffer,30,255,0,0);
    wl_subsurface_set_position(decor->wl_closebutton_subsurface,width - 40,5);
    _wl_decoration_button(decor,&decor->wl_maxbutton_surface,&decor->wl_maxbutton_subsurface,&decor->wl_maxbutton_buffer,30,0,255,0);
    wl_subsurface_set_position(decor->wl_maxbutton_subsurface,width - 75,5);
    _wl_decoration_button(decor,&decor->wl_minbutton_surface,&decor->wl_minbutton_subsurface,&decor->wl_minbutton_buffer,30,0,0,255);
    wl_subsurface_set_position(decor->wl_minbutton_subsurface,width - 110,5);
    return decor;
}

void
wl_decoration_destroy(
    wl_decoration_t *decoration
)
{
    assert(decoration != NULL);
    
    wl_buffer_destroy(decoration->wl_closebutton_buffer);
    wl_subsurface_destroy(decoration->wl_closebutton_subsurface);
    wl_surface_destroy(decoration->wl_closebutton_surface);
        
    wl_buffer_destroy(decoration->wl_minbutton_buffer);
    wl_subsurface_destroy(decoration->wl_minbutton_subsurface);
    wl_surface_destroy(decoration->wl_minbutton_surface);
        
    wl_buffer_destroy(decoration->wl_maxbutton_buffer);
    wl_subsurface_destroy(decoration->wl_maxbutton_subsurface);
    wl_surface_destroy(decoration->wl_maxbutton_surface);

    wl_buffer_destroy(decoration->background_buffer);
    wl_subsurface_destroy(decoration->wl_subsurface);
    wl_surface_destroy(decoration->wl_surface);

    free(decoration);
}

void
wl_decoration_present(
    wl_decoration_t* decoration
)
{
    wl_surface_commit(decoration->wl_surface);
    wl_surface_commit(decoration->wl_closebutton_surface);
    wl_surface_commit(decoration->wl_minbutton_surface);
    wl_surface_commit(decoration->wl_maxbutton_surface);
}


void 
wl_decoration_resize(
    wl_decoration_t* decor,
    uint32_t width,
    const char* title
)
{
    wl_buffer_destroy(decor->background_buffer);
    decor->background_buffer =  _wl_decoration_background(width,title);
    wl_surface_attach(decor->wl_surface,decor->background_buffer,0,0);
    wl_subsurface_set_position(decor->wl_closebutton_subsurface,width - 40,5);
    wl_subsurface_set_position(decor->wl_maxbutton_subsurface,width - 75,5);
    wl_subsurface_set_position(decor->wl_minbutton_subsurface,width - 110,5);
}



#endif /*HAS_WAYLAND*/