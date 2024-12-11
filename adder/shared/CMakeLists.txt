add_library(adrsha STATIC)

target_include_directories(adrsha
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrsha PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sh_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sh_asminfo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sh_program.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sh_arena.c
    ${CMAKE_CURRENT_SOURCE_DIR}/sh_ffi.c
)

target_link_libraries(adrsha PUBLIC m)
target_compile_options(adrsha PRIVATE -Wall -Wextra -Wpedantic)