# add the executable
add_executable(gvm-runner
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.c)

target_link_libraries(gvm-runner PUBLIC m gvm)
target_compile_options(gvm-runner PRIVATE -Wall -Wpedantic -Wextra) #  -Werror

# -mavx2 -msse2 -mfma

target_include_directories(gvm-runner 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

# copy build binaries to bin folder
add_custom_target(install-gvm-runner ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:gvm-runner> ${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:gvm-runner>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources/ ${CMAKE_CURRENT_SOURCE_DIR}/../bin/resources/
)

set_target_properties(
    gvm-runner PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
