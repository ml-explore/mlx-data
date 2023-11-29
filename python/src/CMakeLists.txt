pybind11_add_module(
  _c
  ${CMAKE_CURRENT_LIST_DIR}/wrap.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_core.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_stream.cpp
)

target_include_directories(_c PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(_c PRIVATE mlxdata)

install(TARGETS _c DESTINATION mlx/data)
