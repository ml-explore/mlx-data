cmake_minimum_required(VERSION 3.12...3.18)

# ----------------------------- Setup -----------------------------
project(mlx.data LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ----------------------------- Configuration -----------------------------
option(MLX_ENABLE_AWS "Enable AWS detection" OFF)
option(MLX_BUILD_PYTHON_BINDINGS "Build python bindings for mlx data" OFF)

if(NOT MLX_DATA_VERSION)
  set(MLX_DATA_VERSION 0.0.1)
endif()

# ----------------------------- Lib -----------------------------

if (MLX_BUILD_PYTHON_BINDINGS)
    find_package(
      Python
      COMPONENTS Interpreter Development
      REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
endif()
find_package(SndFile)
find_package(SampleRate)
find_package(Threads REQUIRED)
find_package(ZLIB)
find_package(BZip2)
find_package(LibLZMA)
find_package(FFMPEG COMPONENTS avcodec avdevice avformat avutil swscale)

if(MLX_ENABLE_AWS)
  find_package(AWSSDK COMPONENTS s3)
endif()

find_package(JPEGTURBO)
if(NOT JPEGTURBO_FOUND)
  find_package(JPEG)
endif()

include(CheckSymbolExists)
set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_DL_LIBS})
check_symbol_exists(dlopen "dlfcn.h" HAS_DLOPEN)
check_symbol_exists(RTLD_NOLOAD "dlfcn.h" HAS_RTLD_NOLOAD)
if(APPLE)
  check_symbol_exists(_dyld_get_image_name "mach-o/dyld.h" HAS_DYLD_GET_IMAGE_NAME)
else()
  set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
  check_symbol_exists(dl_iterate_phdr "link.h" HAS_DL_ITERATE_PHDR)
endif()

set(mlxdata-src
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/Array.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/Sample.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/Dataset.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/Buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/Stream.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/BatchShape.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/CSVReader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/FileFetcher.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/Graph.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/Numpy.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/State.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/TARReader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/ThreadController.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/ThreadPool.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/Tokenizer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/Levenshtein.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/Utils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/audio/Audio.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/audio/AudioSndfile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/audio/AudioSampleRate.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/image/ImageTransform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/image/ImageIO.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/image/ImageSTBI.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/image/ImageJPEG.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/video/Video.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/video/VideoFFMPEG.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Buffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Batch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/DynamicBatch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/FromStream.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/FromVector.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/FilesFromTAR.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Partition.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Perm.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Shuffle.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/buffer/Transform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Stream.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Batch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Buffered.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/DynamicBatch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Compose.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/CSVReader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/FromBuffer.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/LineReader.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Partition.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Prefetch.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Repeat.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Shuffle.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/SlidingWindow.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/stream/Transform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Op.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/FilterByShape.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/FilterKey.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/ReadFromTAR.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/KeyTransform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/SampleTransform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Shape.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/RenameKey.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/SaveImage.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/LoadAudio.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/LoadImage.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/LoadFile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/LoadNumpy.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/LoadVideo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Pad.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Shard.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Squeeze.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/Tokenize.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/ImageTransform.cpp
  ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/op/RemoveValue.cpp
)

if(AWSSDK_FOUND)
  list(APPEND mlxdata-src ${CMAKE_CURRENT_LIST_DIR}/lib/mlx/data/core/AWSFileFetcher.cpp)
endif()

add_library(
  mlxdata STATIC ${mlxdata-src})

if(HAS_DLOPEN AND HAS_RTLD_NOLOAD)
  target_compile_definitions(mlxdata PRIVATE MLX_HAS_DLOPEN_NOLOAD)
  target_link_libraries(mlxdata PRIVATE ${CMAKE_DL_LIBS})
endif()

if(HAS_DL_ITERATE_PHDR)
  target_compile_definitions(mlxdata PRIVATE MLX_HAS_DL_ITERATE_PHDR)
  target_link_libraries(mlxdata PRIVATE ${CMAKE_DL_LIBS})
elseif(HAS_DYLD_GET_IMAGE_NAME)
  target_compile_definitions(mlxdata PRIVATE MLX_HAS_DYLD_GET_IMAGE_NAME)
  target_link_libraries(mlxdata PRIVATE ${CMAKE_DL_LIBS})
endif()
target_include_directories(
    mlxdata
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>/lib
    $<INSTALL_INTERFACE:include>
)
set_property(TARGET mlxdata PROPERTY POSITION_INDEPENDENT_CODE ON)
if(SndFile_FOUND)
  target_link_libraries(mlxdata PRIVATE SndFile::sndfile)
  target_compile_definitions(mlxdata PRIVATE MLX_HAS_SNDFILE)
endif()
if(SampleRate_FOUND)
  target_link_libraries(mlxdata PRIVATE SampleRate::samplerate)
  target_compile_definitions(mlxdata PRIVATE MLX_HAS_SAMPLERATE)
endif()
target_link_libraries(mlxdata PRIVATE Threads::Threads)

if(ZLIB_FOUND)
  target_include_directories(mlxdata PUBLIC ${ZLIB_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${ZLIB_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC BXZSTR_Z_SUPPORT=1)
endif()

if(BZIP2_FOUND)
  target_include_directories(mlxdata PUBLIC ${BZIP2_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${BZIP2_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC BXZSTR_BZ2_SUPPORT=1)
endif()

if(LIBLZMA_FOUND)
  target_include_directories(mlxdata PUBLIC ${LIBLZMA_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${LIBLZMA_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC BXZSTR_LZMA_SUPPORT=1)
endif()

if(JPEGTURBO_FOUND)
  target_include_directories(mlxdata PUBLIC ${JPEGTURBO_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${JPEGTURBO_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC MLX_HAS_JPEG)
endif()

if(JPEG_FOUND)
  target_include_directories(mlxdata PUBLIC ${JPEG_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${JPEG_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC MLX_HAS_JPEG)
endif()

if(AWSSDK_FOUND)
  target_include_directories(mlxdata PUBLIC ${AWSSDK_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${AWSSDK_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC MLX_HAS_AWS)
endif()

if (FFMPEG_avformat_FOUND)
  target_include_directories(mlxdata PRIVATE ${FFMPEG_avformat_INCLUDE_DIRS} ${FFMPEG_avdevice_INCLUDE_DIRS} ${FFMPEG_avcodec_INCLUDE_DIRS} ${FFMPEG_avutil_INCLUDE_DIRS} ${FFMPEG_swscale_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE FFMPEG::avformat FFMPEG::avdevice FFMPEG::avcodec FFMPEG::avutil FFMPEG::swscale)
  target_compile_definitions(mlxdata PUBLIC MLX_HAS_FFMPEG)
endif()

include("tests/CMakeLists.txt")

if (MLX_BUILD_PYTHON_BINDINGS)
    include("python/src/CMakeLists.txt")
endif()

# ----------------------------- Installation -----------------------------
include(GNUInstallDirs)

# Install library
install(
    TARGETS mlxdata
    EXPORT MLXDataTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(
    DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/mlx
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT headers
    FILES_MATCHING PATTERN "*.h"
)

# Install cmake config
set(MLX_DATA_CMAKE_BUILD_CONFIG ${CMAKE_BINARY_DIR}/MLXDataConfig.cmake)
set(MLX_DATA_CMAKE_BUILD_VERSION_CONFIG ${CMAKE_BINARY_DIR}/MLXDataConfigVersion.cmake)
set(MLX_DATA_CMAKE_INSTALL_MODULE_DIR share/cmake/MLXData)

install(
  EXPORT MLXDataTargets
  FILE MLXDataTargets.cmake
  DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  ${MLX_DATA_CMAKE_BUILD_VERSION_CONFIG}
  COMPATIBILITY SameMajorVersion
  VERSION ${MLX_DATA_VERSION}
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/mlx-data.pc.in
  ${MLX_DATA_CMAKE_BUILD_CONFIG}
  INSTALL_DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR MLX_DATA_CMAKE_INSTALL_MODULE_DIR
)

install(
  FILES ${MLX_DATA_CMAKE_BUILD_CONFIG} ${MLX_DATA_CMAKE_BUILD_VERSION_CONFIG}
  DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR}
)
