
# add the executable
add_executable(host-example
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c)

target_link_libraries(host-example PUBLIC m adrcom adrvm adrsha xutils)
target_compile_options(host-example PRIVATE -Wall -Wpedantic -Wextra -Werror)

target_include_directories(host-example 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR})

# copy build binaries to bin folder
add_custom_target(install-host-example ALL
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:host-example> ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/$<TARGET_FILE_NAME:host-example>
)

