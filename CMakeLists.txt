cmake_minimum_required(VERSION 3.22)

project("adder" LANGUAGES C)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -ffunction-sections -funwind-tables -fstack-protector-strong -fPIC -Wall -Wformat -Werror=format-security -no-canonical-prefixes -std=c99") # -pg
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -march=native -funroll-loops -ftree-vectorize")

add_subdirectory(adder)
add_subdirectory(adrrun)
add_subdirectory(test-app)

# copy build binaries to bin folder
add_custom_target(install-all ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrcom> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:adrcom>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrvm> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:adrvm>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrsha> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:adrsha>
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrrun> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:adrrun>
)