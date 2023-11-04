cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "-Wextra")

set(CMAKE_C_STANDARD 11)

project("aylin" LANGUAGES C VERSION 0.0.1 DESCRIPTION "Aylin is a window creation library tailor built for wayland")
set(CMAKE_PROJECT_HOMEPAGE_URL "https://github.com/eskpil/aylin")

include("cmake/library.cmake")
include("cmake/examples.cmake")
include("cmake/install.cmake")
