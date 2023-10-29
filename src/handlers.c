#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

#include "aylin.h"
#include "handlers.h"

void _aylin_on_registry_global(void *data, struct wl_registry *registry,
                               uint32_t name, const char *interface,
                               uint32_t version) {
  struct aylin_application *app = data;

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    app->compositor =
        wl_registry_bind(registry, name, &wl_compositor_interface, version);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    app->seat = wl_registry_bind(registry, name, &wl_seat_interface, 4);
  } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
    app->xdg_wm_base =
        wl_registry_bind(registry, name, &xdg_wm_base_interface, version);

  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    app->shm = wl_registry_bind(registry, name, &wl_shm_interface, version);
  } else if (strcmp(interface, wp_presentation_interface.name) == 0) {
    app->presentation =
        wl_registry_bind(registry, name, &wp_presentation_interface, version);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    app->layer_shell = wl_registry_bind(
        registry, name, &zwlr_layer_shell_v1_interface, version);
  } else {
  }
}

void _aylin_on_registry_global_remove(void *data, struct wl_registry *registry,
                                      uint32_t name) {
  // TODO: Implement.
}

void _aylin_on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base,
                                uint32_t serial) {
  xdg_wm_base_pong(wm_base, serial);
}

void _aylin_on_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel) {
  struct aylin_shell *shell = data;

  if (!shell)
    return;

  if (!shell->listener->close) {
    printf("error@1 missing close handler\n");
    exit(EXIT_FAILURE);
  }

  shell->listener->close(shell, shell->_userdata);
}

void _aylin_on_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array *states) {
  struct aylin_shell *shell = data;

  if (width == 0 || height == 0) {
    /* Compositor is deferring to us */
    return;
  }

  shell->width = width;
  shell->height = height;
}

void _aylin_on_xdg_surface_configure(void *data, struct xdg_surface *surface,
                                     uint32_t serial) {
  xdg_surface_ack_configure(surface, serial);

  struct aylin_shell *shell = data;

  struct aylin_shell_frame_event *event = calloc(1, sizeof(*event));
  shell->listener->frame(shell, event, shell->_userdata);
  free(event);
}

void _aylin_on_wl_buffer_release(void *data, struct wl_buffer *wl_buffer) {
  wl_buffer_destroy(wl_buffer);
}

void _aylin_on_presentation_clock_id(void *data,
                                     struct wp_presentation *presentation,
                                     uint32_t clock_id) {
  struct aylin_application *app = data;

  app->clock_id = clock_id;
}

void _aylin_on_wp_feedback_discarded(
    void *data, struct wp_presentation_feedback *wp_feedback) {
  struct aylin_shell *shell = data;

  struct wp_presentation_feedback *feedback =
      wp_presentation_feedback(shell->app->presentation, shell->surface);

  wp_presentation_feedback_add_listener(
      feedback, &_aylin_wp_presentation_feedback_listener, data);
}

void _aylin_on_wp_feedback_presented(
    void *data, struct wp_presentation_feedback *wp_feedback,
    uint32_t tv_sec_hi, uint32_t tv_sec_lo, uint32_t tv_nsec, uint32_t refresh,
    uint32_t seq_hi, uint32_t seq_lo, uint32_t flags) {
  struct aylin_shell *shell = data;

  shell->last_frame_time_nanoseconds =
      (((((uint64_t)tv_sec_hi) << 32) + tv_sec_lo) * 1000000000) + tv_nsec;
  shell->frame_rate = round(1000000000000.0 / refresh);

  // printf("framerate: (%d)\n", shell->frame_rate);

  struct wp_presentation_feedback *feedback =
      wp_presentation_feedback(shell->app->presentation, shell->surface);

  wp_presentation_feedback_add_listener(
      feedback, &_aylin_wp_presentation_feedback_listener, data);
}

void _aylin_on_wp_feedback_sync_output(
    void *data, struct wp_presentation_feedback *feedback,
    struct wl_output *output) {}

void _aylin_on_wl_surface_frame_done(void *data,
                                     struct wl_callback *wl_callback,
                                     uint32_t time) {
  struct aylin_shell *shell = data;

  wl_callback_destroy(wl_callback);

  shell->last_frame_time_nanoseconds = ((uint64_t)time) * 1000000;

  struct wl_callback *callback = wl_surface_frame(shell->surface);
  wl_callback_add_listener(callback, &_aylin_wl_surface_frame_listener, data);

  struct aylin_shell_frame_event *event = calloc(1, sizeof(*event));
  shell->listener->frame(shell, event, shell->_userdata);
  free(event);
}

void _aylin_on_zwlr_surface_closed(
    void *data, struct zwlr_layer_surface_v1 *layer_surface) {
  struct aylin_shell *shell = data;

  if (!shell)
    return;

  if (!shell->listener->close) {
    printf("error@1 missing close handler\n");
    exit(EXIT_FAILURE);
  }

  shell->listener->close(shell, shell->_userdata);
}

void _aylin_on_zwlr_surface_configure(
    void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial,
    uint32_t width, uint32_t height) {
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

  struct aylin_shell *shell = data;

  struct aylin_shell_frame_event *event = calloc(1, sizeof(*event));
  shell->listener->frame(shell, event, shell->_userdata);
  free(event);
}

void _aylin_on_wl_seat_name(void *data, struct wl_seat *seat,
                            const char *name) {}

void _aylin_on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                    uint32_t capabilities) {
  struct aylin_application *app = data;

  app->seat_capabilities = capabilities;
}

void _aylin_on_wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                                 uint32_t serial, struct wl_surface *surface,
                                 struct wl_array *keys) {
  struct aylin_keyboard *keyboard = data;
  struct aylin_shell *shell =
      aylin_application_find_shell_by_surface(keyboard->app, surface);
  assert(shell != NULL);
  keyboard->shell = shell;

  // TODO: Produce keyboard enter callback;
}

void _aylin_on_wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t serial, uint32_t time, uint32_t key,
                               uint32_t state) {
  struct aylin_keyboard *keyboard = data;
  struct aylin_shell *shell = keyboard->shell;
  if (!shell)
    return;

  if (!shell->listener->key_pressed)
    return;

  char buf[128];
  uint32_t keycode = key + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym(keyboard->xkb_state, keycode);
  xkb_keysym_get_name(sym, buf, sizeof(buf));

  enum aylin_input_action action =
      state == WL_KEYBOARD_KEY_STATE_PRESSED ? press : release;

  struct aylin_shell_key_pressed_event *event = calloc(1, sizeof(*event));

  event->serial = serial;
  event->keyboard = keyboard;
  event->keycode = keycode;
  event->symbol = sym;
  event->action = action;

  shell->listener->key_pressed(shell, event, shell->_userdata);

  free(event);
}

void _aylin_on_wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t format, int fd, uint32_t size) {
  struct aylin_keyboard *keyboard = data;
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  const char *map_shm = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(map_shm != MAP_FAILED);

  struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
      keyboard->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);
  munmap((void *)map_shm, size);
  close(fd);

  struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
  xkb_keymap_unref(keyboard->xkb_keymap);
  xkb_state_unref(keyboard->xkb_state);

  keyboard->xkb_keymap = xkb_keymap;
  keyboard->xkb_state = xkb_state;
}

void _aylin_on_wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                                 uint32_t serial, struct wl_surface *surface) {
  struct aylin_keyboard *keyboard = data;
  keyboard->shell = NULL;

  // TODO: Produce keyboard leave callback;
}

void _aylin_on_wl_keyboard_modifiers(void *data,
                                     struct wl_keyboard *wl_keyboard,
                                     uint32_t serial, uint32_t mods_depressed,
                                     uint32_t mods_latched,
                                     uint32_t mods_locked, uint32_t group) {
  struct aylin_keyboard *keyboard = data;
  struct aylin_shell *shell = keyboard->shell;
  if (!shell)
    return;

  if (!shell->listener->keyboard_modifiers)
    return;

  struct aylin_shell_keyboard_modifiers_event *event =
      calloc(1, sizeof(*event));

  event->keyboard = keyboard;

  event->serial = serial;
  event->mods_depressed = mods_depressed;
  event->mods_latched = mods_latched;
  event->mods_locked = mods_locked;
  event->group = group;

  free(event);
}

void _aylin_on_wl_keyboard_repeat_info(void *data, struct wl_keyboard *keyboard,
                                       int rate, int delay) {}

void _aylin_on_wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                                uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t surface_x, wl_fixed_t surface_y) {
  struct aylin_pointer *pointer = data;
  pointer->shell =
      aylin_application_find_shell_by_surface(pointer->app, surface);

  if (!pointer->shell)
    return;

  if (!pointer->shell->listener->pointer_motion)
    return;

  double x = wl_fixed_to_double(surface_x);
  double y = wl_fixed_to_double(surface_y);

  pointer->x = x;
  pointer->y = y;

  struct aylin_shell_pointer_motion_event *event = calloc(1, sizeof(*event));

  event->pointer = pointer;
  event->serial = serial;
  event->x = x;
  event->y = y;

  pointer->shell->listener->pointer_motion(pointer->shell, event,
                                           pointer->shell->_userdata);

  free(event);
}

void _aylin_on_wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                                uint32_t serial, struct wl_surface *surface) {}

void _aylin_on_wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, wl_fixed_t surface_x,
                                 wl_fixed_t surface_y) {
  struct aylin_pointer *pointer = data;
  if (!pointer->shell)
    return;

  if (!pointer->shell->listener->pointer_motion)
    return;

  double x = wl_fixed_to_double(surface_x);
  double y = wl_fixed_to_double(surface_y);

  pointer->x = x;
  pointer->y = y;

  struct aylin_shell_pointer_motion_event *event = calloc(1, sizeof(*event));

  event->pointer = pointer;
  event->serial = UINT32_MAX;
  event->x = x;
  event->y = y;

  pointer->shell->listener->pointer_motion(pointer->shell, event,
                                           pointer->shell->_userdata);

  free(event);
}

void _aylin_on_wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t serial, uint32_t time,
                                 uint32_t button, uint32_t state) {
  struct aylin_pointer *pointer = data;
  if (!pointer->shell)
    return;

  if (!pointer->shell->listener->pointer_button)
    return;

  struct aylin_shell_pointer_button_event *event = calloc(1, sizeof(*event));

  event->pointer = pointer;

  event->serial = serial;
  event->button = button;

  event->x = pointer->x;
  event->y = pointer->y;

  event->action = state;

  pointer->shell->listener->pointer_button(pointer->shell, event,
                                           pointer->shell->_userdata);

  free(event);
}

void _aylin_on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                               uint32_t time, uint32_t axis, wl_fixed_t value) {
  struct aylin_pointer *pointer = data;
  if (!pointer->shell)
    return;

  if (!pointer->shell->listener->pointer_axis)
    return;

  struct aylin_shell_pointer_axis_event *event = calloc(1, sizeof(*event));

  event->pointer = pointer;

  event->axis = axis;
  event->value = wl_fixed_to_double(value);

  pointer->shell->listener->pointer_axis(pointer->shell, event,
                                         pointer->shell->_userdata);

  free(event);
}
