
# SHARED (INSIDE ADRCOM) 

add_library(adrcomshared STATIC)

target_include_directories(adrcomshared
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomshared PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/co_program.c
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/co_utils.c
)

target_link_libraries(adrcomshared PUBLIC m adrsha)
target_compile_options(adrcomshared PRIVATE -Wall -Wpedantic -Wextra -Werror)

# PARSER

add_library(adrcomparse STATIC)

target_include_directories(adrcomparse
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomparse PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/co_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/parser/co_tokenizer.c
)

target_link_libraries(adrcomparse PUBLIC m adrsha)
target_compile_options(adrcomparse PRIVATE -Wall -Wpedantic -Wextra -Werror)

# TYPE CHECKER

add_library(adrcomcheck STATIC)

target_include_directories(adrcomcheck
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomcheck PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/typechecker/co_bty.c
)

target_link_libraries(adrcomcheck PUBLIC m adrsha adrcomshared)
target_compile_options(adrcomcheck PRIVATE -Wall -Wpedantic -Wextra -Werror)

# COMPILER

add_library(adrcomcompiler STATIC)

target_include_directories(adrcomcompiler
PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(adrcomcompiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler/co_compiler.c
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler/co_srcmap.c
)

target_link_libraries(adrcomcompiler PUBLIC m adrsha)
target_compile_options(adrcomcompiler PRIVATE -Wall -Wpedantic -Wextra -Werror)


# ADDER COMPILER BUNDLE

add_library(adrcom STATIC 
    "$<TARGET_OBJECTS:adrcomshared>"
    "$<TARGET_OBJECTS:adrcomparse>"
    "$<TARGET_OBJECTS:adrcomcheck>"
    "$<TARGET_OBJECTS:adrcomcompiler>"
)