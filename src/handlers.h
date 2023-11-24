#ifndef AYLIN_HANDLERS_H_
#define AYLIN_HANDLERS_H_

#include <wayland-client.h>

#include "protocols/presentation-time-client-protocol.h"
#include "protocols/wlr-layer-shell-unstable-v1-client-protocol.h"
#include "protocols/xdg-shell-client-protocol.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void _aylin_on_registry_global(void *data, struct wl_registry *registry,
                               uint32_t name, const char *interface,
                               uint32_t version);
void _aylin_on_registry_global_remove(void *data, struct wl_registry *registry,
                                      uint32_t name);

static const struct wl_registry_listener _aylin_registry_handler = {
    .global = _aylin_on_registry_global,
    .global_remove = _aylin_on_registry_global_remove,
};

void _aylin_on_xdg_wm_base_ping(void *data, struct xdg_wm_base *wm_base,
                                uint32_t serial);

static const struct xdg_wm_base_listener _aylin_xdg_wm_base_listener = {
    .ping = _aylin_on_xdg_wm_base_ping,
};

void _aylin_on_xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array *states);

void _aylin_on_xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel);

static const struct xdg_toplevel_listener _aylin_xdg_toplevel_listener = {
    .configure = _aylin_on_xdg_toplevel_configure,
    .close = _aylin_on_xdg_toplevel_close,
};

void _aylin_on_xdg_surface_configure(void *data, struct xdg_surface *surface,
                                     uint32_t serial);

static const struct xdg_surface_listener _aylin_xdg_surface_listener = {
    .configure = _aylin_on_xdg_surface_configure,
};

void _aylin_on_wl_buffer_release(void *data, struct wl_buffer *wl_buffer);

static const struct wl_buffer_listener _aylin_wl_buffer_listener = {
    .release = _aylin_on_wl_buffer_release,
};

void _aylin_on_presentation_clock_id(void *data,
                                     struct wp_presentation *presentation,
                                     uint32_t clock_id);

static const struct wp_presentation_listener _aylin_wp_presentation_listener = {
    .clock_id = _aylin_on_presentation_clock_id,
};

void _aylin_on_wp_feedback_discarded(void *data,
                                     struct wp_presentation_feedback *feedback);
void _aylin_on_wp_feedback_presented(void *data,
                                     struct wp_presentation_feedback *feedback,
                                     uint32_t tv_sec_hi, uint32_t tv_sec_lo,
                                     uint32_t tv_nsec, uint32_t refresh,
                                     uint32_t seq_hi, uint32_t seq_lo,
                                     uint32_t flags);
void _aylin_on_wp_feedback_sync_output(
    void *data, struct wp_presentation_feedback *feedback,
    struct wl_output *output);

static const struct wp_presentation_feedback_listener
    _aylin_wp_presentation_feedback_listener = {
        .discarded = _aylin_on_wp_feedback_discarded,
        .presented = _aylin_on_wp_feedback_presented,
        .sync_output = _aylin_on_wp_feedback_sync_output,
};

void _aylin_on_wl_surface_frame_done(void *data, struct wl_callback *callback,
                                     uint32_t callback_data);

static const struct wl_callback_listener _aylin_wl_surface_frame_listener = {
    .done = _aylin_on_wl_surface_frame_done,
};

void _aylin_on_zwlr_surface_closed(void *data,
                                   struct zwlr_layer_surface_v1 *layer_surface);
void _aylin_on_zwlr_surface_configure(
    void *data, struct zwlr_layer_surface_v1 *layer_surface, uint32_t serial,
    uint32_t width, uint32_t height);

static const struct zwlr_layer_surface_v1_listener
    _aylin_zwlr_surface_listener = {
        .closed = _aylin_on_zwlr_surface_closed,
        .configure = _aylin_on_zwlr_surface_configure,
};

void _aylin_on_wl_seat_name(void *data, struct wl_seat *seat, const char *name);
void _aylin_on_wl_seat_capabilities(void *data, struct wl_seat *seat,
                                    uint32_t capabilities);

static const struct wl_seat_listener _aylin_wl_seat_listener = {
    .name = _aylin_on_wl_seat_name,
    .capabilities = _aylin_on_wl_seat_capabilities,
};

void _aylin_on_wl_keyboard_enter(void *data, struct wl_keyboard *keyboard,
                                 uint32_t serial, struct wl_surface *surface,
                                 struct wl_array *keys);
void _aylin_on_wl_keyboard_key(void *data, struct wl_keyboard *keyboard,
                               uint32_t serial, uint32_t time, uint32_t key,
                               uint32_t state);
void _aylin_on_wl_keyboard_keymap(void *data, struct wl_keyboard *keyboard,
                                  uint32_t format, int fd, uint32_t size);
void _aylin_on_wl_keyboard_leave(void *data, struct wl_keyboard *keyboard,
                                 uint32_t serial, struct wl_surface *surface);
void _aylin_on_wl_keyboard_modifiers(void *data, struct wl_keyboard *keyboard,
                                     uint32_t serial, uint32_t mods_depressed,
                                     uint32_t mods_latched,
                                     uint32_t mods_locked, uint32_t group);
void _aylin_on_wl_keyboard_repeat_info(void *data, struct wl_keyboard *keyboard,
                                       int rate, int delay);

static const struct wl_keyboard_listener _aylin_wl_keyboard_listener = {
    .enter = _aylin_on_wl_keyboard_enter,
    .key = _aylin_on_wl_keyboard_key,
    .keymap = _aylin_on_wl_keyboard_keymap,
    .leave = _aylin_on_wl_keyboard_leave,
    .modifiers = _aylin_on_wl_keyboard_modifiers,
    .repeat_info = _aylin_on_wl_keyboard_repeat_info,
};

void _aylin_on_wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                                uint32_t serial, struct wl_surface *surface,
                                wl_fixed_t surface_x, wl_fixed_t surface_y);
void _aylin_on_wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                                uint32_t serial, struct wl_surface *surface);
void _aylin_on_wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, wl_fixed_t surface_x,
                                 wl_fixed_t surface_y);
void _aylin_on_wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t serial, uint32_t time,
                                 uint32_t button, uint32_t state);
void _aylin_on_wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                               uint32_t time, uint32_t axis, wl_fixed_t value);

static const struct wl_pointer_listener _aylin_wl_pointer_listener = {
    .enter = _aylin_on_wl_pointer_enter,
    .leave = _aylin_on_wl_pointer_leave,
    .motion = _aylin_on_wl_pointer_motion,
    .button = _aylin_on_wl_pointer_button,
    .axis = _aylin_on_wl_pointer_axis,
};

void _aylin_on_wl_output_geometry(void *data, struct wl_output *wl_output,
                                  int32_t x, int32_t y, int32_t physical_width,
                                  int32_t physical_height, int32_t subpixel,
                                  const char *make, const char *model,
                                  int32_t transform);

void _aylin_on_wl_output_mode(void *data, struct wl_output *wl_output,
                              uint32_t flags, int32_t width, int32_t height,
                              int32_t refresh);

void _aylin_on_wl_output_done(void *data, struct wl_output *wl_output);

void _aylin_on_wl_output_scale(void *data, struct wl_output *wl_output,
                               int32_t factor);

void _aylin_on_wl_output_name(void *data, struct wl_output *wl_output,
                              const char *name);

void _aylin_on_wl_output_description(void *data, struct wl_output *wl_output,
                                     const char *description);

static const struct wl_output_listener _aylin_wl_output_listener = {
    .geometry = _aylin_on_wl_output_geometry,
    .mode = _aylin_on_wl_output_mode,
    .done = _aylin_on_wl_output_done,
    .scale = _aylin_on_wl_output_scale,
    .description = _aylin_on_wl_output_description,
    .name = _aylin_on_wl_output_name,
};

void _aylin_on_xdg_popup_configure(void *data, struct xdg_popup *xdg_popup,
                                   int32_t x, int32_t y, int32_t width,
                                   int32_t height);

void _aylin_on_xdg_popup_done(void *data, struct xdg_popup *xdg_popup);
void _aylin_on_xdg_popup_repositioned(void *data, struct xdg_popup *xdg_popup,
                                      uint32_t token);

static const struct xdg_popup_listener _aylin_xdg_popup_listener = {
    .configure = _aylin_on_xdg_popup_configure,
    .popup_done = _aylin_on_xdg_popup_done,
    .repositioned = _aylin_on_xdg_popup_repositioned,
};

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // AYLIN_HANDLERS_H_
