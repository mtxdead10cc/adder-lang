add_library(adrcom STATIC)

target_include_directories(adrcom
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/co_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_compiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_tokenizer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_srcmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_program.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/co_bty.c
)

target_link_libraries(adrcom PUBLIC m adrsha)
target_compile_options(adrcom PRIVATE -Wall -Wpedantic -Wextra -Werror)

