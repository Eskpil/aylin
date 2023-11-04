cmake_minimum_required(VERSION 3.10)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/aylin.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/aylin.pc"
        @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/aylin.pc"
        DESTINATION lib/pkgconfig
)

install(TARGETS ${LIBAYLIN}
        LIBRARY DESTINATION lib
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
        DESTINATION include/aylin
        FILES_MATCHING PATTERN "*.h"
)