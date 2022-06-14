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


#include "../config.h"
#include "../color.h"
#include "wl_backend.h"
#include "wl_window.h"
#include "wl_target.h"

typedef struct surface_impl_wl_t {
  impl_type_t type;
  struct wl_buffer *wl_buffer;
  struct wl_surface *wl_surface;
} surface_impl_wl_t;

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

surface_impl_wl_t *
surface_create_wl_impl(
  wl_target_t *wl_target,
  int32_t width,
  int32_t height,
  color_t_ **data)
{
  assert(wl_target != NULL);
  assert(wl_target->wl_shm != NULL);
  assert(wl_target->wl_surface != NULL);
  assert(width > 0);
  assert(height  > 0);
  assert(data != NULL);
  assert(*data == NULL);

  surface_impl_wl_t *impl =
    (surface_impl_wl_t *)calloc(1, sizeof(surface_impl_wl_t));
  if (impl == NULL) {
    return NULL;
  }

  // _surface_create_wl_image(..., width, height, &s->data, ...);

  uint32_t shm_pool_size = width * height * 4;
  int fd = _allocate_shm_file(shm_pool_size);
  uint8_t *pool_data = (uint8_t *)mmap(NULL, shm_pool_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0);

  struct wl_shm_pool *pool = wl_shm_create_pool(wl_target->wl_shm,
                                                fd, shm_pool_size);
  close(fd); // fd no longer needed at this point

  struct wl_buffer *wl_buffer =
    wl_shm_pool_create_buffer(pool, 0, width, height, width * 4,
                              WL_SHM_FORMAT_ARGB8888); // or ARGB for alpha

// or pool could be kept...
  wl_shm_pool_destroy(pool); // the buffer keeps a reference to the pool so it's ok to destroy

  impl->type = IMPL_WAYLAND;
  impl->wl_buffer = wl_buffer;
  impl->wl_surface = wl_target->wl_surface;
  *data = (color_t_ *)pool_data;
  wl_surface_attach(impl->wl_surface, impl->wl_buffer, 0, 0);
  wl_surface_damage(impl->wl_surface,0,0,width,height);
  wl_surface_commit(impl->wl_surface);

  return impl;
}

void
surface_destroy_wl_impl(
  surface_impl_wl_t *impl)
{
  assert(impl != NULL);
  assert(impl->type == IMPL_WAYLAND);

  wl_buffer_destroy(impl->wl_buffer);
  wl_surface_destroy(impl->wl_surface);

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
surface_resize_wl_impl(
  wl_target_t *wl_target,
  surface_impl_wl_t *impl,
  int32_t s_width,
  int32_t s_height,
  color_t_ **s_data,
  int32_t d_width,
  int32_t d_height,
  color_t_ **d_data)
{
  assert(wl_target != NULL);
  assert(impl != NULL);
  assert(s_width > 0);
  assert(s_height  > 0);
  assert(s_data != NULL);
  assert(*s_data != NULL);
  assert(d_width > 0);
  assert(d_height  > 0);
  assert(d_data != NULL);
  assert(*d_data == NULL);


  //Destroy old buffer
  wl_buffer_destroy(impl->wl_buffer);
  //Create a new shared memory slot with the correct size
  uint32_t shm_pool_size = d_width * d_height * 4;
  int fd = _allocate_shm_file(shm_pool_size);
  uint8_t *pool_data = (uint8_t *)mmap(NULL, shm_pool_size,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED, fd, 0);

  struct wl_shm_pool *pool = wl_shm_create_pool(wl_target->wl_shm,
                                                fd, shm_pool_size);
  close(fd); 
  //Create buffer and get data handle
  impl->wl_buffer = wl_shm_pool_create_buffer(pool,0,d_width,d_height,d_width*4,WL_SHM_FORMAT_ARGB8888);
  *d_data = (color_t_ *)pool_data;
  //Copy data
  _raw_surface_copy(*s_data,s_width,s_height,*d_data,d_width,d_height);
  //Delete old data
  munmap(s_data,s_width * s_height * 4);
  return true;
}

void
surface_present_wl_impl(
  surface_impl_wl_t *impl,
  int32_t width,
  int32_t height,
  wl_present_data_t *present_data)
{
  assert(impl != NULL);
  assert(present_data != NULL);
  assert(width > 0);
  assert(height > 0);
  wl_surface_attach(impl->wl_surface, impl->wl_buffer, 0, 0);
  wl_surface_damage(impl->wl_surface,0,0,width,height);
  wl_surface_commit(impl->wl_surface);
}

#else

const int wl_surface = 0;

#endif /* HAS_WAYLAND */
