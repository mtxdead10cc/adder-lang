add_library(adrsha)

target_include_directories(adrsha
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc)

target_sources(adrsha PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gvm_asminfo.c
)

target_link_libraries(adrsha PUBLIC m)
target_compile_options(adrsha PRIVATE -Wall -Wextra -Wpedantic)