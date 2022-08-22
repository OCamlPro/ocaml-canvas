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

#include <stdint.h>

#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>

#include <wayland-client.h>


#include "../color.h"
#include "wl_backend_internal.h"

static void
_randname(
  char *buf)
{
  struct timespec ts = { 0 };
  clock_gettime(CLOCK_REALTIME, &ts);
  long r = ts.tv_nsec;
  for (int i = 0; i < 6; ++i) {
    buf[i] = 'A' + (r & 15) + (r & 16) * 2;
    r >>= 5;
  }
}

static int
_create_shm_file()
{
  int retries = 100;
  char name[] = "/canvas_shm-XXXXXX";

  do {
    _randname(name + sizeof(name) - 7);
    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR | O_CLOEXEC, 0600);
    if (fd >= 0) {
      shm_unlink(name);
      return fd;
    }
  } while ((--retries > 0) && (errno == EEXIST));

  return -1;
}

static int
_allocate_shm_file(
  size_t size)
{
  int fd = _create_shm_file();

  if (fd < 0) {
    return -1;
  }

  int ret = 0;
  do {
    ret = posix_fallocate(fd, 0, size);
  } while ((ret < 0) && (errno == EINTR));

  if (ret < 0) {
    close(fd);
    return -1;
  }

  return fd;
}

struct wl_buffer *
wl_create_buffer(
  int32_t width,
  int32_t height,
  uint8_t **data)
{
  assert(width > 0);
  assert(height > 0);
  assert(data != NULL);
  assert(*data == NULL);

//TODO: add error handling

  size_t shm_pool_size = (size_t)(width * height * COLOR_SIZE);
  int fd = _allocate_shm_file(shm_pool_size);
  *data = (uint8_t *)mmap(NULL, shm_pool_size,
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

  struct wl_shm_pool *pool =
    wl_shm_create_pool(wl_back->shm, fd, (int)shm_pool_size);

  close(fd);

  struct wl_buffer *wl_buffer =
    wl_shm_pool_create_buffer(pool, 0, width, height, width * COLOR_SIZE,
                              WL_SHM_FORMAT_ARGB8888); // or ARGB for alpha
  wl_shm_pool_destroy(pool);

  return wl_buffer;
}

#endif /* HAS_WAYLAND */
