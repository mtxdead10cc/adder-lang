
add_library(gvm)

target_include_directories(gvm
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(gvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_heap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_value.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_env.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_asmutils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_compiler.c
)


target_link_libraries(gvm PUBLIC m)
target_compile_options(gvm PRIVATE -Wall -Wextra -Wpedantic)