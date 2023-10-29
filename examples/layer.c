#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "aylin.h"

#include <cairo/cairo.h>

static void on_frame(struct aylin_shell *shell,
                     struct aylin_shell_frame_event *event, void *data) {
  struct aylin_buffer *buffer = aylin_shell_create_buffer(shell);

  cairo_surface_t *cairo_surface = aylin_buffer_create_cairo(buffer);
  cairo_t *cr = cairo_create(cairo_surface);

  cairo_set_source_rgba(cr, 0.694, 0.384, 0.525, 1.0);
  cairo_paint(cr);

  cairo_fill(cr);
  cairo_destroy(cr);

  aylin_destroy_buffer(buffer);
}

static void on_key_pressed(struct aylin_shell *shell,
                           struct aylin_shell_key_pressed_event *event,
                           void *data) {}

static void on_pointer_motion(struct aylin_shell *shell,
                              struct aylin_shell_pointer_motion_event *event,
                              void *data) {}

static void on_pointer_button(struct aylin_shell *shell,
                              struct aylin_shell_pointer_button_event *event,
                              void *data) {}

static void on_pointer_axis(struct aylin_shell *shell,
                            struct aylin_shell_pointer_axis_event *event,
                            void *data) {}

static void on_close(struct aylin_shell *shell, void *data) {
  // aylin_application_terminate forces the event loop to terminate giving us
  // the ability to free up all allocating resources gracefully.
  //
  // it is safe to terminate the application before we destroy the shell since
  // termination happens on the next event loop run. And we can assume the next
  // event loop run will be after the current execution context is finished.
  aylin_application_terminate(shell->app);

  // even though you draw CSD the compositor may still choose to close us. So
  // this is where you would insert your application logic to persist valuable
  // state. For example, open a dialog asking the user to save or discard the
  // unsaved document.
  aylin_shell_destroy(shell);
}

static const struct aylin_shell_listener shell_listener = {
    .frame = on_frame,
    .key_pressed = on_key_pressed,
    .pointer_motion = on_pointer_motion,
    .pointer_button = on_pointer_button,
    .pointer_axis = on_pointer_axis,
    .close = on_close,
};

int main() {
  // listener and userdata is optional on the application. Since not every
  // application instance may want to receive information about outputs and such
  // alike. You could think of it this way, every event sent to the application
  // listener communicates global compositor state and not surface specific
  // state.
  struct aylin_application *app =
      aylin_application_create("io.aylin.example", NULL, NULL);
  if (app == NULL) {
    fprintf(stderr, "[Error]: Could not create application: (%s)\n",
            strerror(errno));
    return 1;
  }

  struct aylin_shell *shell = aylin_layer_create(app, &shell_listener, NULL);
  if (shell == NULL) {
    fprintf(stderr, "[Error]: Could not create layer: (%s)\n", strerror(errno));
    return 1;
  }

  aylin_shell_set_dimensions(shell, 1920, 40);
  aylin_layer_set_anchor(shell, aylin_shell_anchor_bottom);
  aylin_layer_set_exclusivity_zone(shell, shell->height);

  int ret = aylin_application_poll(app);
  if (0 > ret) {
    fprintf(stderr, "[Error]: Could not poll: (%s)\n", strerror(errno));
    return 1;
  }

  aylin_application_destroy(app);

  return 0;
}