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

#include "wl_decoration.h"
#include "wl_backend.h"
#include "wl_backend_internal.h"



//TODO : Factor this between surface and decoration
static void
_randname(
  char *buf)
{
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A'+(r&15)+(r&16)*2;
    r >>= 5;
  }
}
//TODO : Factor this between surface and decoration
static int
_create_shm_file()
{
  int retries = 100;
  do {
    char name[] = "/canvas_shm-XXXXXX";
    _randname(name + sizeof(name) - 7);
    --retries;
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while (retries > 0 && errno == EEXIST);
  return -1;
}
//TODO : Factor this between surface and decoration
static int
_allocate_shm_file(
  size_t size)
{
  int fd = _create_shm_file();
  if (fd < 0)
    return -1;
  int ret;
  do {
    ret = ftruncate(fd, size);
  } while (ret < 0 && errno == EINTR);
  if (ret < 0) {
    close(fd);
    return -1;
  }
  return fd;
}



wl_decoration_t*
wl_decoration_create(
    struct wl_surface *parent,
    uint32_t width
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

    //TODO : Factor this between surface and decoration
    uint32_t shm_pool_size = width * _decor_height * 4;
    int fd = _allocate_shm_file(shm_pool_size);
    uint8_t *pool_data = (uint8_t *)mmap(NULL, shm_pool_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0);

    struct wl_shm_pool *pool = wl_shm_create_pool(wl_back->shm,
                                                fd, shm_pool_size);
    close(fd); 

    struct wl_buffer *wl_buffer =
    wl_shm_pool_create_buffer(pool, 0, width, _decor_height, width * 4,
                              WL_SHM_FORMAT_ARGB8888); // or ARGB for alpha
    wl_shm_pool_destroy(pool);

    //TODO : Improve the design
    for (int i = 0;i < width*_decor_height;i++)
    {
        pool_data[4*i] = 0;
        pool_data[4*i + 1] = 0;
        pool_data[4*i + 2] = 0;
        pool_data[4*i + 3] = 255;
    }

    munmap(pool_data,shm_pool_size);

    wl_surface_attach(decor->wl_surface,wl_buffer,0,0);
    wl_subsurface_set_position(decor->wl_subsurface,0,-_decor_height);
    decor->background_buffer = wl_buffer;

    return decor;
}

void
wl_decoration_destroy(
    wl_decoration_t *decoration
)
{
    assert(decoration != NULL);
    assert(decoration->background_buffer != NULL);
    assert(decoration->wl_subsurface != NULL);
    assert(decoration->wl_surface != NULL);

    wl_buffer_destroy(decoration->background_buffer);
    wl_subsurface_destroy(decoration->wl_subsurface);
    wl_surface_destroy(decoration->wl_surface);
}

void
wl_decoration_present(
    wl_decoration_t* decoration
)
{
    wl_surface_commit(decoration->wl_surface);
}


#endif /*HAS_WAYLAND*/