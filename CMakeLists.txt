cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_FLAGS "-Wextra")

set(CMAKE_C_STANDARD 11)
project("aylin" LANGUAGES C)

include("cmake/library.cmake")
include("cmake/examples.cmake")
