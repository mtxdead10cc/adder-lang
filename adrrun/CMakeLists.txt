
add_custom_target(gen-test-header
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test/gen/generate.py
    COMMENT "Generate langtest.h"
)

# add the executable
add_executable(adrrun
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.c)

target_link_libraries(adrrun PUBLIC m adrcom adrvm adrsha)
target_compile_options(adrrun PRIVATE -Wall -Wpedantic -Wextra) #  -Werror
add_dependencies(adrrun gen-test-header)

# -mavx2 -msse2 -mfma

target_include_directories(adrrun 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})


# copy build binaries to bin folder
add_custom_target(install-adrrun ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adrrun> ${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:adrrun>
)

