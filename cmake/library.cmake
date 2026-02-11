cmake_minimum_required(VERSION 3.10)

find_package(PkgConfig)
# requires for supporting keyboard inputs.
pkg_check_modules(XKBCOMMON REQUIRED xkbcommon)

pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor)
pkg_check_modules(CAIRO REQUIRED cairo)

include(cmake/generate_wayland_protocols.cmake)

pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
pkg_get_variable(WLROOTS_PROTOCOLS_DATADIR wlr-protocols pkgdatadir)

set(_wayland_protocols_xml_dir "${WAYLAND_PROTOCOLS_DATADIR}")
set(_wlroots_protocols_xml_dir "${WLROOTS_PROTOCOLS_DATADIR}")

set(_wayland_protocols_src_dir "${CMAKE_CURRENT_SOURCE_DIR}/src/protocols")
file(MAKE_DIRECTORY "${_wayland_protocols_src_dir}")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/staging/cursor-shape/cursor-shape-v1.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/cursor-shape-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/cursor-shape-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/stable/tablet/tablet-v2.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/tablet-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/tablet-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/xdg-decoration.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/xdg-decoration-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/stable/xdg-shell/xdg-shell.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/xdg-shell-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/xdg-shell-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/text-input/text-input-unstable-v1.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v1-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v1-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/unstable/text-input/text-input-unstable-v3.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v3-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/text-input-unstable-v3-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wayland_protocols_xml_dir}/stable/presentation-time/presentation-time.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/presentation-time-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/presentation-time-client-protocol.h")

generate_wayland_client_protocol(
        PROTOCOL_FILE "${_wlroots_protocols_xml_dir}/unstable/wlr-layer-shell-unstable-v1.xml"
        CODE_FILE "${_wayland_protocols_src_dir}/wlr-layer-shell-v1-protocol.c"
        HEADER_FILE "${_wayland_protocols_src_dir}/wlr-layer-shell-unstable-v1-client-protocol.h")

set(SOURCES
        "src/aylin.c"
        "src/handlers.c"
        "src/buffer.c"
        "src/shm.c"

        "${_wayland_protocols_src_dir}/xdg-shell-protocol.c"
        "${_wayland_protocols_src_dir}/text-input-unstable-v1-protocol.c"
        "${_wayland_protocols_src_dir}/text-input-unstable-v3-protocol.c"
        "${_wayland_protocols_src_dir}/presentation-time-protocol.c"
        "${_wayland_protocols_src_dir}/wlr-layer-shell-v1-protocol.c"
        "${_wayland_protocols_src_dir}/cursor-shape-protocol.c"
        "${_wayland_protocols_src_dir}/tablet-protocol.c"
        "${_wayland_protocols_src_dir}/xdg-decoration.c"
)

set(LIBAYLIN "aylin")
add_library(
        ${LIBAYLIN}
        SHARED
        ${SOURCES}
)

target_link_libraries(
        ${LIBAYLIN}
        m
        ${WAYLAND_PROTOCOLS_LIBRARIES}
        ${WAYLAND_CLIENT_LIBRARIES}
        ${WAYLAND_CURSOR_LIBRARIES}
        ${XKBCOMMON_LIBRARIES}
        ${CAIRO_LIBRARIES}
)

target_include_directories(
        ${LIBAYLIN}
        PUBLIC
        "src"
        "src/protocols"
)

