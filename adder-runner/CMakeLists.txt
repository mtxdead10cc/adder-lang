
add_custom_target(gen-test-header
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test/gen/generate.py
    COMMENT "Generate langtest.h"
)

# add the executable
add_executable(adder-runner
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.c)

target_link_libraries(adder-runner PUBLIC m adder)
target_compile_options(adder-runner PRIVATE -Wall -Wpedantic -Wextra) #  -Werror
add_dependencies(adder-runner gen-test-header)

# -mavx2 -msse2 -mfma

target_include_directories(adder-runner 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})


# copy build binaries to bin folder
add_custom_target(install-adder-runner ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:adder-runner> ${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:adder-runner>
)

