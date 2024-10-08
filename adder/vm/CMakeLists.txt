
add_library(adrvm)

target_include_directories(adrvm
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(adrvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm_heap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm_value_tools.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm_env.c
)

target_link_libraries(adrvm PUBLIC m adrsha adrcom)
target_compile_options(adrvm PRIVATE -Wall -Wextra -Wpedantic)