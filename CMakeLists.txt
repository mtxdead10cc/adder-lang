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

# copy build binaries to bin folder
#add_custom_target(install-all ALL
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrcom> ${CMAKE_CURRENT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:adrcom>
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrvm> ${CMAKE_CURRENT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:adrvm>
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrsha> ${CMAKE_CURRENT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:adrsha>
#    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrrun> ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:adrrun>
#)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" ARCH_NAME)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/adder/" # source directory
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_NAME}/" # target directory
    FILES_MATCHING # install only matched files
    PATTERN "*.h" # select header files
)

install(
    FILES $<TARGET_FILE:adrcom>
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_NAME}/compiler"
)

install(
    FILES $<TARGET_FILE:adrvm>
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_NAME}/vm"
)

install(
    FILES $<TARGET_FILE:adrsha>
    DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/${ARCH_NAME}/shared"
)