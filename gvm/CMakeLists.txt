
add_library(gvm)

target_include_directories(gvm
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(gvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_value.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_memory.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_asm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_grid.c
)


target_link_libraries(gvm PUBLIC m)
target_compile_options(gvm PRIVATE -Wall -Wextra -Wpedantic)