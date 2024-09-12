# add the executable
add_executable(test-app
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/board/board.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_runner.c)

target_link_libraries(test-app PUBLIC m gvm)
target_compile_options(test-app PRIVATE -Wall -Wpedantic -Wextra) #  -Werror

# -mavx2 -msse2 -mfma

target_include_directories(test-app 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

# copy build binaries to bin folder
add_custom_target(install-test-app ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:test-app> ${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<TARGET_FILE_NAME:test-app>
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources/ ${CMAKE_CURRENT_SOURCE_DIR}/../bin/resources/
)

set_target_properties(
    test-app PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
