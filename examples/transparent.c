#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "aylin.h"

#include <cairo/cairo.h>

static void draw_rectangle(cairo_t *cr, double x, double y, double width,
                           double height, double aspect, double corner_radius) {
  double radius = corner_radius / aspect;
  double degrees = M_PI / 180.0;

  cairo_new_sub_path(cr);
  cairo_arc(cr, x + width - radius, y + radius, radius, -90 * degrees,
            0 * degrees);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0 * degrees,
            90 * degrees);
  cairo_arc(cr, x + radius, y + height - radius, radius, 90 * degrees,
            180 * degrees);
  cairo_arc(cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
  cairo_close_path(cr);

  cairo_fill_preserve(cr);
  cairo_stroke(cr);
}

static void on_frame(struct aylin_shell *shell,
                     struct aylin_shell_frame_event *event, void *userdata) {
  struct aylin_buffer *buffer = aylin_shell_create_buffer(shell);

  cairo_surface_t *cairo_surface = aylin_buffer_create_cairo(buffer);
  cairo_t *cr = cairo_create(cairo_surface);

  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.5, 0.5);
  draw_rectangle(cr, 0, 0, buffer->width, buffer->height, 1.0, 12.0);

  cairo_destroy(cr);
  cairo_surface_destroy(cairo_surface);

  aylin_destroy_buffer(buffer);
}

// NOTE: Be aware, the wl_pointer.enter event actually sends us the coordinates
// of the pointer from where it is in the surface at entry. pointer_enter_events
// should also take this into consideration.
// struct aylin_shell_pointer_enter_event contains the pointer coordinates of
// the pointer.
static void on_pointer_enter(struct aylin_shell *shell,
                             struct aylin_shell_pointer_enter_event *event,
                             void *data) {}

static void on_pointer_leave(struct aylin_shell *shell,
                             struct aylin_shell_pointer_leave_event *event,
                             void *data) {}

static void on_key_pressed(struct aylin_shell *shell,
                           struct aylin_shell_key_pressed_event *event,
                           void *data) {}

static void on_pointer_motion(struct aylin_shell *shell,
                              struct aylin_shell_pointer_motion_event *event,
                              void *data) {}

static void on_pointer_button(struct aylin_shell *shell,
                              struct aylin_shell_pointer_button_event *event,
                              void *data) {

  if (event->action == press && event->button == left) {
    aylin_window_move(shell, event->serial);
  }
}

static void on_pointer_axis(struct aylin_shell *shell,
                            struct aylin_shell_pointer_axis_event *event,
                            void *data) {}

static void on_resize(struct aylin_shell *shell,
                      struct aylin_shell_resize_event *event, void *data) {
  switch (event->action) {
  case resize:
    printf("resizing to: (%dx%d)\n", event->width, event->height);
    break;
  case fullscreen:
    printf("full-screening to: (%dx%d)\n", event->width, event->height);
    break;
  case maximize:
    printf("maximizing to: (%dx%d)\n", event->width, event->height);
    break;
  default:
    break;
  }
}

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
    .close = on_close,
    .frame = on_frame,
    .resize = on_resize,

    .key_pressed = on_key_pressed,

    .pointer_motion = on_pointer_motion,
    .pointer_button = on_pointer_button,
    .pointer_axis = on_pointer_axis,
    .pointer_enter = on_pointer_enter,
    .pointer_leave = on_pointer_leave,
};

static void on_output(struct aylin_application *app,
                      struct aylin_output *output, void *data) {
  //  printf("output: %dx%d (%s) %s\n", output->width, output->height,
  //  output->name,
  //         output->description);
}

static void on_process(struct aylin_application *app, void *data) {
  //  printf("process callback\n");
}

static const struct aylin_application_listener app_listener = {
    .output = on_output,
    .process = on_process,
};

int main() {
  // listener and userdata is optional on the application. Since not every
  // application instance may want to receive information about outputs and such
  // alike. You could think of it this way, every event sent to the application
  // listener communicates global compositor state and not surface specific
  // state.
  struct aylin_application *app =
      aylin_application_create("io.aylin.example", &app_listener, NULL);
  if (app == NULL) {
    fprintf(stderr, "[Error]: Could not create application: (%s)\n",
            strerror(errno));
    return 1;
  }

  struct aylin_shell *shell = aylin_window_create(app, &shell_listener, NULL);
  if (shell == NULL) {
    fprintf(stderr, "[Error]: Could not create window: (%s)\n",
            strerror(errno));
    return 1;
  }

  aylin_window_set_title(shell, "example");

  int ret = aylin_application_poll(app);
  if (0 > ret) {
    fprintf(stderr, "[error]: could not poll: (%s)\n", strerror(errno));
  }

  // gracefully destroy the application and all its remaining resources
  aylin_application_destroy(app);

  return 0;
}
