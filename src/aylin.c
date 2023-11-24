#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aylin.h"
#include "handlers.h"

#define MAX_EVENTS 128

static struct aylin_keyboard *
aylin_application_create_keyboard(struct aylin_application *app) {
  struct aylin_keyboard *keyboard = calloc(1, sizeof(*keyboard));

  keyboard->app = app;
  keyboard->wl_keyboard = wl_seat_get_keyboard(app->seat);
  keyboard->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  wl_keyboard_add_listener(keyboard->wl_keyboard, &_aylin_wl_keyboard_listener,
                           keyboard);

  return keyboard;
}

static void aylin_keyboard_destroy(struct aylin_keyboard *keyboard) {
  wl_keyboard_destroy(keyboard->wl_keyboard);

  xkb_context_unref(keyboard->xkb_context);
  xkb_keymap_unref(keyboard->xkb_keymap);
  xkb_state_unref(keyboard->xkb_state);

  free(keyboard);
}

static struct aylin_pointer *
aylin_application_create_pointer(struct aylin_application *app) {
  struct aylin_pointer *pointer = calloc(1, sizeof(*pointer));

  pointer->app = app;
  pointer->wl_pointer = wl_seat_get_pointer(app->seat);

  wl_pointer_add_listener(pointer->wl_pointer, &_aylin_wl_pointer_listener,
                          pointer);

  return pointer;
}

static void aylin_pointer_destroy(struct aylin_pointer *pointer) {
  wl_pointer_destroy(pointer->wl_pointer);

  free(pointer);
}

static struct aylin_touch *
aylin_application_create_touch(struct aylin_application *app) {
  struct aylin_touch *touch = calloc(1, sizeof(*touch));

  // TODO: Implement touch

  return touch;
}

static void aylin_touch_destroy(struct aylin_touch *touch) { free(touch); }

static void aylin_application_initialize_input(struct aylin_application *app) {
  if (app->seat_capabilities & WL_SEAT_CAPABILITY_KEYBOARD)
    app->keyboard = aylin_application_create_keyboard(app);

  if (app->seat_capabilities & WL_SEAT_CAPABILITY_POINTER)
    app->pointer = aylin_application_create_pointer(app);

  if (app->seat_capabilities & WL_SEAT_CAPABILITY_TOUCH)
    app->touch = aylin_application_create_touch(app);
}

struct aylin_application *
aylin_application_create(char *app_id,
                         const struct aylin_application_listener *listener,
                         void *_userdata) {
  struct aylin_application *app = calloc(1, sizeof(*app));

  wl_list_init(&app->shells);
  wl_list_init(&app->outputs);

  app->terminated = false;

  app->listener = listener;
  app->_userdata = _userdata;

  app->epollfd = epoll_create1(0);
  if (app->epollfd == -1) {
    return NULL;
  }

  app->display = wl_display_connect(NULL);
  if (!app->display) {
    return NULL;
  }

  int display_fd = wl_display_get_fd(app->display);

  struct epoll_event ev = {
      .data.fd = display_fd,
      .events = EPOLLIN,
  };
  if (epoll_ctl(app->epollfd, EPOLL_CTL_ADD, display_fd, &ev) == -1) {
    return NULL;
  }

  app->registry = wl_display_get_registry(app->display);

  wl_registry_add_listener(app->registry, &_aylin_registry_handler, app);
  wl_display_roundtrip(app->display);

  wl_seat_add_listener(app->seat, &_aylin_wl_seat_listener, app);
  xdg_wm_base_add_listener(app->xdg_wm_base, &_aylin_xdg_wm_base_listener, app);
  wp_presentation_add_listener(app->presentation,
                               &_aylin_wp_presentation_listener, app);

  // wl_seat events.
  wl_display_roundtrip(app->display);

  aylin_application_initialize_input(app);

  app->app_id = strdup(app_id);

  return app;
}

void _aylin_application_create_output(struct aylin_application *app,
                                      struct wl_output *wl_output) {
  struct aylin_output *output = calloc(1, sizeof(*output));
  output->wl_output = wl_output;
  output->app = app;

  wl_output_add_listener(output->wl_output, &_aylin_wl_output_listener, output);

  wl_list_insert(&app->outputs, &output->link);
}

struct wl_surface *
aylin_application_create_independent_surface(struct aylin_application *app) {
  return wl_compositor_create_surface(app->compositor);
}

void aylin_application_destroy_independent_surface(struct wl_surface *surface) {
  wl_surface_destroy(surface);
}

int aylin_application_poll(struct aylin_application *app) {
  int display_fd = wl_display_get_fd(app->display);
  if (0 > display_fd) {
    return display_fd;
  }

  while (!app->terminated) {
    wl_display_flush(app->display);

    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(app->epollfd, events, MAX_EVENTS, 10);
    if (nfds == -1) {
      return nfds;
    }

    for (int n = 0; nfds > n; ++n) {
      if (events[n].data.fd == display_fd) {
        while (wl_display_prepare_read(app->display) != 0) {
          int ret = wl_display_dispatch_pending(app->display);
          if (ret == -1) {
            return ret;
          }
        }

        int ret = wl_display_read_events(app->display);
        if (ret == -1) {
          return ret;
        }

        ret = wl_display_dispatch_pending(app->display);
        if (ret == -1) {
          return ret;
        }
      }
    }

    if (app->listener->process)
      app->listener->process(app, app->_userdata);
  }

  return 0;
}

struct aylin_shell *
aylin_application_find_shell_by_surface(struct aylin_application *app,
                                        struct wl_surface *surface) {
  struct aylin_shell *shell, *tmp;
  wl_list_for_each_safe(shell, tmp, &app->shells, link) {
    if (shell->surface == surface) {
      return shell;
    }
  }

  return NULL;
}

void aylin_application_terminate(struct aylin_application *app) {
  app->terminated = true;
}

void aylin_application_destroy(struct aylin_application *app) {
  struct aylin_shell *shell, *tmp;
  wl_list_for_each_safe(shell, tmp, &app->shells, link) {
    aylin_shell_destroy(shell);
  }

  if (app->pointer)
    aylin_pointer_destroy(app->pointer);
  if (app->keyboard)
    aylin_keyboard_destroy(app->keyboard);
  if (app->touch)
    aylin_touch_destroy(app->touch);

  xdg_wm_base_destroy(app->xdg_wm_base);
  zwlr_layer_shell_v1_destroy(app->layer_shell);
  wp_presentation_destroy(app->presentation);
  wl_shm_destroy(app->shm);
  wl_seat_destroy(app->seat);
  wl_compositor_destroy(app->compositor);
  wl_registry_destroy(app->registry);
  wl_display_disconnect(app->display);

  free(app);
}

static struct aylin_shell *
aylin_shell_create_base(struct aylin_application *app,
                        const struct aylin_shell_listener *listener,
                        void *_userdata) {
  struct aylin_shell *shell = calloc(1, sizeof(*shell));

  shell->_userdata = _userdata;
  shell->listener = listener;

  shell->app = app;

  shell->surface = aylin_application_create_independent_surface(app);
  if (!shell->surface) {
    return NULL;
  }

  wl_list_insert(&app->shells, &shell->link);

  return shell;
}

static void aylin_shell_frame(struct aylin_shell *shell) {
  struct wl_callback *callback = wl_surface_frame(shell->surface);
  wl_callback_add_listener(callback, &_aylin_wl_surface_frame_listener, shell);

  if (shell->app->presentation) {
    struct wp_presentation_feedback *feedback =
        wp_presentation_feedback(shell->app->presentation, shell->surface);

    wp_presentation_feedback_add_listener(
        feedback, &_aylin_wp_presentation_feedback_listener, shell);
  }
}

struct wl_surface *aylin_shell_get_surface(struct aylin_shell *shell) {
  return shell->surface;
}

uint32_t aylin_shell_get_frame_rate(struct aylin_shell *shell) {
  return shell->frame_rate;
}

struct aylin_shell *
aylin_window_create(struct aylin_application *app,
                    const struct aylin_shell_listener *listener, void *data) {

  struct aylin_shell *shell = aylin_shell_create_base(app, listener, data);
  assert(shell != NULL);

  shell->kind = AYLIN_SHELL_KIND_XDG;

  shell->width = 720;
  shell->height = 480;

  shell->xdg.surface =
      xdg_wm_base_get_xdg_surface(app->xdg_wm_base, shell->surface);
  if (!shell->xdg.surface) {
    return NULL;
  }

  xdg_surface_add_listener(shell->xdg.surface, &_aylin_xdg_surface_listener,
                           shell);

  shell->xdg.toplevel = xdg_surface_get_toplevel(shell->xdg.surface);
  if (!shell->xdg.surface) {
    return NULL;
  }

  xdg_toplevel_add_listener(shell->xdg.toplevel, &_aylin_xdg_toplevel_listener,
                            shell);

  wl_surface_commit(shell->surface);
  wl_display_roundtrip(app->display);

  xdg_toplevel_set_app_id(shell->xdg.toplevel, app->app_id);

  aylin_shell_frame(shell);

  wl_display_roundtrip(app->display);

  return shell;
}

void aylin_window_set_title(struct aylin_shell *shell, char *title) {
  if (shell->xdg.title) {
    free(shell->xdg.title);
  }

  shell->xdg.title = strdup(title);
  xdg_toplevel_set_title(shell->xdg.toplevel, shell->xdg.title);
}

void aylin_window_move(struct aylin_shell *window, uint32_t serial) {
  assert(window->kind == AYLIN_SHELL_KIND_XDG);
  xdg_toplevel_move(window->xdg.toplevel, window->app->seat, serial);
}

struct aylin_shell *
aylin_layer_create(struct aylin_application *app,
                   const struct aylin_shell_listener *listener,
                   void *_userdata) {
  struct aylin_shell *shell = aylin_shell_create_base(app, listener, _userdata);
  assert(shell != NULL);

  shell->width = 1920;
  shell->height = 48;

  shell->kind = AYLIN_SHELL_KIND_LAYER;

  shell->layer.surface = zwlr_layer_shell_v1_get_layer_surface(
      shell->app->layer_shell, shell->surface, NULL,
      ZWLR_LAYER_SHELL_V1_LAYER_TOP, app->app_id);
  if (!shell->layer.surface) {
    return NULL;
  }

  zwlr_layer_surface_v1_add_listener(shell->layer.surface,
                                     &_aylin_zwlr_surface_listener, shell);

  wl_surface_commit(shell->surface);

  aylin_shell_frame(shell);

  return shell;
}

void aylin_shell_set_dimensions(struct aylin_shell *shell, int32_t width,
                                int32_t height) {
  shell->width = width;
  shell->height = height;

  if (shell->kind == AYLIN_SHELL_KIND_XDG) {
    xdg_surface_set_window_geometry(shell->xdg.surface, 0, 0, width, height);
  } else if (shell->kind == AYLIN_SHELL_KIND_LAYER) {
    zwlr_layer_surface_v1_set_size(shell->layer.surface, width, height);
  } else {
    printf("unknown shell kind");
    exit(1);
  }
}

void aylin_layer_set_anchor(struct aylin_shell *shell,
                            enum aylin_shell_anchor anchor) {
  assert(shell->kind == AYLIN_SHELL_KIND_LAYER);
  zwlr_layer_surface_v1_set_anchor(shell->layer.surface, anchor);
  wl_surface_commit(shell->surface);
}

void aylin_layer_set_exclusivity_zone(struct aylin_shell *shell, int32_t zone) {
  assert(shell->kind == AYLIN_SHELL_KIND_LAYER);
  zwlr_layer_surface_v1_set_exclusive_zone(shell->layer.surface, zone);
}

struct aylin_positioner *
aylin_shell_create_positioner(struct aylin_shell *shell) {
  struct aylin_positioner *positioner = calloc(1, sizeof(*positioner));

  positioner->positioner =
      xdg_wm_base_create_positioner(shell->app->xdg_wm_base);

  return positioner;
}

void aylin_positioner_set_size(struct aylin_positioner *positioner, int width,
                               int height) {
  assert(positioner);
  xdg_positioner_set_size(positioner->positioner, width, height);
}

void aylin_positioner_set_anchor(struct aylin_positioner *positioner,
                                 enum aylin_positioner_anchor anchor) {
  assert(positioner);
  xdg_positioner_set_anchor(positioner->positioner, anchor);
}

void aylin_positioner_set_gravity(struct aylin_positioner *positioner,
                                  enum aylin_positioner_gravity gravity) {
  assert(positioner);
  xdg_positioner_set_gravity(positioner->positioner, gravity);
}

void aylin_positioner_set_constraint_adjustment(
    struct aylin_positioner *positioner, uint32_t constraint_adjustment) {
  assert(positioner);
  xdg_positioner_set_constraint_adjustment(positioner->positioner,
                                           constraint_adjustment);
}

void aylin_positioner_set_anchor_rect(struct aylin_positioner *positioner,
                                      int x, int y, int width, int height) {
  assert(positioner);
  xdg_positioner_set_anchor_rect(positioner->positioner, x, y, width, height);
}

// parent is allowed to be NULL.
struct aylin_shell *
aylin_popup_create(struct aylin_application *app, struct aylin_shell *parent,
                   struct aylin_positioner *positioner,
                   const struct aylin_shell_listener *listener,
                   void *userdata) {
  struct aylin_shell *popup = aylin_shell_create_base(app, listener, userdata);

  popup->kind = AYLIN_SHELL_KIND_XDG_POPUP;

  struct xdg_surface *parent_surface = NULL;
  if (parent && parent->kind == AYLIN_SHELL_KIND_XDG) {
    parent_surface = parent->xdg.surface;
  }

  popup->xdg_popup.surface =
      xdg_wm_base_get_xdg_surface(app->xdg_wm_base, popup->surface);
  if (popup->xdg_popup.surface == NULL)
    return NULL;

  xdg_surface_add_listener(popup->xdg_popup.surface,
                           &_aylin_xdg_surface_listener, popup);

  popup->xdg_popup.popup = xdg_surface_get_popup(
      popup->xdg_popup.surface, parent_surface, positioner->positioner);
  if (popup->xdg_popup.popup == NULL)
    return NULL;

  xdg_popup_add_listener(popup->xdg_popup.popup, &_aylin_xdg_popup_listener,
                         popup);

  wl_surface_commit(popup->surface);
  wl_display_roundtrip(app->display);

  aylin_shell_frame(popup);

  return popup;
}

void aylin_shell_destroy(struct aylin_shell *shell) {
  wl_list_remove(&shell->link);

  switch (shell->kind) {
  case AYLIN_SHELL_KIND_LAYER:
    zwlr_layer_surface_v1_destroy(shell->layer.surface);
    break;
  case AYLIN_SHELL_KIND_XDG:
    xdg_toplevel_destroy(shell->xdg.toplevel);
    xdg_surface_destroy(shell->xdg.surface);
    if (shell->xdg.title)
      free(shell->xdg.title);
    break;
  case AYLIN_SHELL_KIND_SESSION_LOCK:
  default:
    printf("unreachable\n");
    exit(EXIT_FAILURE);
  }

  aylin_application_destroy_independent_surface(shell->surface);

  free(shell);
}
