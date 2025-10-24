
# SHARED (ALL)

add_library(adrsha STATIC)

target_include_directories(adrsha
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrsha PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_log.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_utils.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_asminfo.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_program.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_arena.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_ffi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/sh_ift.c
)

target_link_libraries(adrsha PUBLIC m)
target_compile_options(adrsha PRIVATE -Wall -Wpedantic -Wextra -Werror)

# ADRVM

add_library(adrvm STATIC)

target_include_directories(adrvm
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vm/vm.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm/vm_heap.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm/vm_value_tools.c
        ${CMAKE_CURRENT_SOURCE_DIR}/vm/vm_env.c
)

target_link_libraries(adrvm PUBLIC m adrsha)
target_compile_options(adrvm PRIVATE -Wall -Wpedantic -Wextra -Werror)

# XUTILS

add_library(xutils STATIC)

target_include_directories(xutils
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(xutils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/xutils/xu_lib.c
    ${CMAKE_CURRENT_SOURCE_DIR}/xutils/xu_invoke.c
)

target_link_libraries(xutils PRIVATE adrsha adrcom adrvm)
target_compile_options(xutils PRIVATE -Wall -Wpedantic -Wextra -Werror)


# ADRCOM/SHARED

add_library(adrcomshared STATIC)

target_include_directories(adrcomshared
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomshared PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/shared/co_utils.c
)

target_link_libraries(adrcomshared PUBLIC m adrsha)
target_compile_options(adrcomshared PRIVATE -Wall -Wpedantic -Wextra -Werror)

# ADRCOM/PARSER

add_library(adrcomparse STATIC)

target_include_directories(adrcomparse
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomparse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/parser/co_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/parser/co_tokenizer.c
)

target_link_libraries(adrcomparse PUBLIC m adrsha)
target_compile_options(adrcomparse PRIVATE -Wall -Wpedantic -Wextra -Werror)

# ADRCOM/TYPE CHECKER

add_library(adrcomcheck STATIC)

target_include_directories(adrcomcheck
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomcheck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/typechecker/co_bty.c
)

target_link_libraries(adrcomcheck PUBLIC m adrsha adrcomshared)
target_compile_options(adrcomcheck PRIVATE -Wall -Wpedantic -Wextra -Werror)

# ADRCOM/COMPILER

add_library(adrcomcompiler STATIC)

target_include_directories(adrcomcompiler
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomcompiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/compiler/co_compiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/compiler/co_srcmap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/adrcom/compiler/co_program.c
)

target_link_libraries(adrcomcompiler PUBLIC m adrsha)
target_compile_options(adrcomcompiler PRIVATE -Wall -Wpedantic -Wextra -Werror)


# ADRCOM/ADRCOM (BUNDLE)

add_library(adrcom STATIC 
    "$<TARGET_OBJECTS:adrcomshared>"
    "$<TARGET_OBJECTS:adrcomparse>"
    "$<TARGET_OBJECTS:adrcomcheck>"
    "$<TARGET_OBJECTS:adrcomcompiler>"
)