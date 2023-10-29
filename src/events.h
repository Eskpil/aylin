#ifndef AYLIN_EVENTS_H_
#define AYLIN_EVENTS_H_

#include <linux/input-event-codes.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

enum aylin_pointer_axis {
  vertical_scroll,
  horizontal_scroll,
};

enum aylin_pointer_button {
  mouse = BTN_MOUSE,
  left = BTN_LEFT,
  right = BTN_RIGHT,
  middle = BTN_MIDDLE,
  side = BTN_SIDE,
  extra = BTN_EXTRA,
  back = BTN_BACK,
  task = BTN_TASK,
};

enum aylin_input_action {
  press,
  release,
};

struct aylin_shell_key_pressed_event {
  struct aylin_keyboard *keyboard;
  uint32_t serial, keycode, symbol;
  enum aylin_input_action action;
};

struct aylin_shell_keyboard_modifiers_event {
  struct aylin_keyboard *keyboard;
  uint32_t serial, mods_depressed, mods_latched, mods_locked, group;
};

struct aylin_shell_pointer_motion_event {
  struct aylin_pointer *pointer;
  uint32_t serial;
  double x, y;
};

struct aylin_shell_pointer_button_event {
  struct aylin_pointer *pointer;
  uint32_t serial;
  double x, y;
  enum aylin_input_action action;
  enum aylin_pointer_button button;
};

struct aylin_shell_pointer_axis_event {
  struct aylin_pointer *pointer;
  double value;
  enum aylin_pointer_axis axis;
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AYLIN_EVENTS_H_
