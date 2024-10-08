
add_custom_target(gen-test-header
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/test/gen/generate.py
    COMMENT "Generate Headers"
)

# add the executable
add_executable(gvm-runner
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.c)

target_link_libraries(gvm-runner PUBLIC m gvm)
target_compile_options(gvm-runner PRIVATE -Wall -Wpedantic -Wextra) #  -Werror
add_dependencies(gvm-runner gen-test-header)

# -mavx2 -msse2 -mfma

target_include_directories(gvm-runner 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})


# copy build binaries to bin folder
add_custom_target(install-gvm-runner ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gvm-runner> ${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:gvm-runner>
)

