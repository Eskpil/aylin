#ifndef AYLIN_H_
#define AYLIN_H_

#include <stdbool.h>
#include <sys/epoll.h>

#include <cairo/cairo.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <xkbcommon/xkbcommon.h>

#include "events.h"

#include "protocols/presentation-time-client-protocol.h"
#include "protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "protocols/xdg-shell-client-protocol.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct aylin_application {
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct wl_seat *seat;
  struct wl_shm *shm;
  struct wp_presentation *presentation;
  struct zwlr_layer_shell_v1 *layer_shell;
  struct xdg_wm_base *xdg_wm_base;

  char *app_id;

  uint32_t clock_id, seat_capabilities;

  struct wl_list shells;
  struct wl_list outputs;

  struct aylin_keyboard *keyboard;
  struct aylin_pointer *pointer;
  struct aylin_touch *touch;

  const struct aylin_application_listener *listener;
  void *_userdata;

  bool terminated;

  int epollfd;
};

struct aylin_keyboard {
  struct wl_keyboard *wl_keyboard;
  struct aylin_application *app;
  struct aylin_shell *shell;

  struct xkb_state *xkb_state;
  struct xkb_context *xkb_context;
  struct xkb_keymap *xkb_keymap;
};

struct aylin_pointer {
  struct wl_pointer *wl_pointer;
  struct aylin_application *app;
  struct aylin_shell *shell;

  double x, y;
};

struct aylin_touch {
  struct wl_touch *touch;
  struct aylin_application *app;
  struct aylin_shell *shell;
};

enum aylin_shell_kind { xdg, layer, session_lock };
enum aylin_shell_anchor {
  aylin_shell_anchor_top = 1,
  aylin_shell_anchor_bottom = 2,
  aylin_shell_anchor_left = 4,
  aylin_shell_anchor_right = 8,
};

struct aylin_shell {
  struct wl_list link; // aylin_application::shells;
  struct aylin_application *app;

  struct wl_surface *surface;

  enum aylin_shell_kind kind;

  union {
    struct {
      struct xdg_toplevel *toplevel;
      struct xdg_surface *surface;

      char *title;
    } xdg;
    struct {
      struct zwlr_layer_surface_v1 *surface;
    } layer;
    struct {
    } session_lock;
  };

  uint32_t frame_rate;
  uint64_t last_frame_time_nanoseconds;

  int32_t width, height;

  const struct aylin_shell_listener *listener;
  void *_userdata;
};

struct aylin_buffer {
  int fd;

  struct aylin_shell *shell;

  struct wl_shm_pool *pool;
  struct wl_buffer *wl_buffer;

  uint8_t *bytes;

  int32_t width, height, size, stride;
};

struct aylin_output {
  struct wl_list link; // aylin_application::outputs
  struct aylin_application *app;

  struct wl_output *wl_output;

  int32_t x, y, physical_width, physical_height, subpixel, transform, scale,
      width, height, refresh;
  uint32_t flags;

  char *name, *make, *model, *description;
};

struct aylin_application_listener {
  void (*output)(struct aylin_application *app, struct aylin_output *output,
                 void *data);
};

struct aylin_shell_listener {
  void (*close)(struct aylin_shell *shell, void *data);
  void (*pointer_axis)(struct aylin_shell *shell,
                       struct aylin_shell_pointer_axis_event *event,
                       void *data);
  void (*pointer_button)(struct aylin_shell *shell,
                         struct aylin_shell_pointer_button_event *event,
                         void *data);
  void (*pointer_motion)(struct aylin_shell *shell,
                         struct aylin_shell_pointer_motion_event *event,
                         void *data);
  void (*keyboard_modifiers)(struct aylin_shell *shell,
                             struct aylin_shell_keyboard_modifiers_event *event,
                             void *data);
  void (*key_pressed)(struct aylin_shell *shell,
                      struct aylin_shell_key_pressed_event *event, void *data);
  void (*frame)(struct aylin_shell *window,
                struct aylin_shell_frame_event *event, void *data);
};

struct aylin_application *
aylin_application_create(char *app_id,
                         const struct aylin_application_listener *listener,
                         void *userdata);
int aylin_application_poll(struct aylin_application *app);

struct aylin_shell *
aylin_application_find_shell_by_surface(struct aylin_application *app,
                                        struct wl_surface *surface);

struct wl_surface *
aylin_application_create_independent_surface(struct aylin_application *app);
void aylin_application_destroy_independent_surface(struct wl_surface *surface);

void _aylin_application_create_output(struct aylin_application *app,
                                      struct wl_output *wl_output);

void aylin_application_terminate(struct aylin_application *app);

void aylin_application_destroy(struct aylin_application *app);

// ----------------------- shell ---------------------------------------

void aylin_shell_set_dimensions(struct aylin_shell *window, int32_t width,
                                int32_t height);

struct wl_surface *aylin_shell_get_surface(struct aylin_shell *shell);

uint32_t aylin_shell_get_frame_rate(struct aylin_shell *shell);

void aylin_shell_destroy(struct aylin_shell *shell);

// ----------------------- window --------------------------------------

struct aylin_shell *
aylin_window_create(struct aylin_application *app,
                    const struct aylin_shell_listener *listener,
                    void *userdata);

void aylin_window_set_title(struct aylin_shell *window, char *title);

// ----------------------- layer --------------------------------------

struct aylin_shell *
aylin_layer_create(struct aylin_application *app,
                   const struct aylin_shell_listener *listener, void *userdata);

void aylin_layer_set_anchor(struct aylin_shell *shell,
                            enum aylin_shell_anchor anchor);

void aylin_layer_set_exclusivity_zone(struct aylin_shell *shell, int32_t zone);

// ----------------------- buffer -------------------------------------

struct aylin_buffer *aylin_shell_create_buffer(struct aylin_shell *window);

cairo_surface_t *aylin_buffer_create_cairo(struct aylin_buffer *buffer);

void aylin_destroy_buffer(struct aylin_buffer *buffer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AYLIN_H_
