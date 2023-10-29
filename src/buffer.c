#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "aylin.h"
#include "handlers.h"
#include "shm.h"

struct aylin_buffer *aylin_shell_create_buffer(struct aylin_shell *shell) {
  struct aylin_buffer *buffer = calloc(1, sizeof(*buffer));

  buffer->shell = shell;

  buffer->height = shell->height;
  buffer->width = shell->width;

  buffer->stride = buffer->width * 4;
  buffer->size = buffer->stride * buffer->height;

  buffer->fd = allocate_shm_file(buffer->size);

  buffer->bytes = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                       buffer->fd, 0);

  return buffer;
}

cairo_surface_t *aylin_buffer_create_cairo(struct aylin_buffer *buffer) {
  cairo_surface_t *cr = cairo_image_surface_create_for_data(
      buffer->bytes, CAIRO_FORMAT_ARGB32, buffer->width, buffer->height,
      buffer->stride);
  return cr;
}

void aylin_destroy_buffer(struct aylin_buffer *buffer) {
  struct aylin_shell *shell = buffer->shell;

  buffer->pool = wl_shm_create_pool(shell->app->shm, buffer->fd, buffer->size);
  buffer->wl_buffer =
      wl_shm_pool_create_buffer(buffer->pool, 0, buffer->width, buffer->height,
                                buffer->stride, WL_SHM_FORMAT_XRGB8888);
  wl_shm_pool_destroy(buffer->pool);

  wl_surface_attach(shell->surface, buffer->wl_buffer, 0, 0);
  wl_surface_damage_buffer(shell->surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit(shell->surface);

  wl_buffer_add_listener(buffer->wl_buffer, &_aylin_wl_buffer_listener, NULL);

  close(buffer->fd);

  munmap(buffer->bytes, buffer->size);
  free(buffer);
}
