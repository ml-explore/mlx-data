# A self-contained runtime check for ArrowStream: it builds its own
# ArrowArrayStream, so it needs no test framework and no Arrow C++. Off by
# default, like the python bindings.
option(MLX_BUILD_TESTS "Build mlx data tests" OFF)
if(MLX_BUILD_TESTS)
  enable_testing()
  add_executable(test_arrow_stream
                 ${CMAKE_CURRENT_LIST_DIR}/test_arrow_stream.cpp)
  # The test declares its own ArrowArray and ArrowArrayStream, so it needs
  # nanoarrow's headers; the symbols are already inside mlxdata.
  target_link_libraries(test_arrow_stream PRIVATE mlxdata)
  target_include_directories(
    test_arrow_stream PRIVATE ${CMAKE_CURRENT_LIST_DIR}/.. ${nanoarrow-include})
  add_test(NAME arrow_stream COMMAND test_arrow_stream)
endif()
