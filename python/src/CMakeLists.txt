find_package(
  Python
  COMPONENTS Interpreter Development.Module
  REQUIRED)
execute_process(
  COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
  OUTPUT_STRIP_TRAILING_WHITESPACE
  OUTPUT_VARIABLE pybind11_ROOT)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(
  _c
  ${CMAKE_CURRENT_LIST_DIR}/wrap.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_core.cpp
  ${CMAKE_CURRENT_LIST_DIR}/wrap_stream.cpp)

target_include_directories(_c PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(_c PRIVATE mlxdata)
target_compile_definitions(_c PRIVATE _VERSION_=${MLX_DATA_VERSION})

install(TARGETS _c DESTINATION mlx/data)
