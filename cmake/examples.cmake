cmake_minimum_required(VERSION 3.10)

add_executable(
        "window-example"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/window.c"
)

target_link_libraries(
        "window-example"
        ${LIBAYLIN}
)

target_include_directories(
        "window-example"
        PUBLIC
        "src"
)

add_executable(
        "layer-example"
        "${CMAKE_CURRENT_SOURCE_DIR}/examples/layer.c"
)

target_link_libraries(
        "layer-example"
        ${LIBAYLIN}
)

target_include_directories(
        "layer-example"
        PUBLIC
        "src"
)