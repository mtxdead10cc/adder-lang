
add_library(adder)

target_include_directories(adder
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adder PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_heap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_value.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_env.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_parser.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_compiler.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_tokenizer.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gvm_srcmap.c
)


target_link_libraries(adder PUBLIC m)
target_compile_options(adder PRIVATE -Wall -Wextra -Wpedantic)