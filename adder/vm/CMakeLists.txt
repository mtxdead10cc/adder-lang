
add_library(adrvm STATIC)

target_include_directories(adrvm
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm_heap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm_value_tools.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm_env.c
)

target_link_libraries(adrvm PUBLIC m adrsha)
target_compile_options(adrvm PRIVATE -Wall -Wpedantic -Wextra -Werror)