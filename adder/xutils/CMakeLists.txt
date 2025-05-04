add_library(xutils STATIC)

target_include_directories(xutils
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(xutils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/xu_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/xu_invoke.c
)

target_link_libraries(xutils PRIVATE adrsha adrcom adrvm)
target_compile_options(xutils PRIVATE -Wall -Wpedantic -Wextra -Werror)