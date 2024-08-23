cmake_minimum_required(VERSION 3.22)

project("gvm" LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra") # -pg
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -march=native -funroll-loops -ftree-vectorize")

add_subdirectory(gvm)
add_subdirectory(test-app)