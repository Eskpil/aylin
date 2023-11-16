# Aylin

Aylin is a wayland based window creation library. It currently supports the xdg_shell and zwlr_layer_shell protocols.
With ext_session_lock being planned.

### Todos

- Handle touch input
- ext_session_lock

### Dependencies

To build Aylin your system needs these packages in the pkg-config search path

- `wayland-client`
- `wayland-cursor`
- `xkbcommon`
- `wayland-protocols`
- `wlr-protocols`

### Build

```shell
mkdir build
cd build

cmake -G=Ninja ..
ninja
```

### Examples

By default, Aylin builds the two built in examples.

- layer-example (**zwlr_layer_shell**)
- window-example (**xdg_shell**)
