cmake_minimum_required(VERSION 3.16)

# ----------------------------- Setup -----------------------------
project(mlx.data LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# ----------------------------- Configuration -----------------------------
option(MLX_BUILD_PYTHON_BINDINGS "Build python bindings for mlx data" OFF)

if(NOT MLX_DATA_VERSION)
  set(MLX_DATA_VERSION 0.1.0)
endif()

# ----------------------------- 3rdparty -----------------------------

include(FetchContent)

# stb
FetchContent_Declare(
  stb
  GIT_REPOSITORY "https://github.com/nothings/stb.git"
  GIT_TAG "f0569113c93ad095470c54bf34a17b36646bbbb5")
FetchContent_MakeAvailable(stb)
add_library(stb INTERFACE) # because no CMake file in stb
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

# bxzstr
FetchContent_Declare(
  bxzstr
  GIT_REPOSITORY "https://github.com/tmaklin/bxzstr.git"
  GIT_TAG "v1.2.3"
  PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bxzstr-v1.2.3.patch
  # never update (thus never patch again)
  UPDATE_COMMAND "")
FetchContent_MakeAvailable(bxzstr)

# ----------------------------- lib -----------------------------

find_package(SndFile)
find_package(SampleRate)
find_package(Threads REQUIRED)
find_package(FFMPEG COMPONENTS avcodec avdevice avformat avutil swscale
                               swresample)
find_package(ZLIB) # AWS SDK CMake misses this one
find_package(
  AWSSDK
  COMPONENTS s3
  QUIET)
if(AWSSDK_FOUND)
  message(
    STATUS
      "AWS SDK found (includes: ${AWSSDK_INCLUDE_DIRS} libs: ${AWSSDK_LIBRARIES})"
  )
else()
  message(STATUS "Could NOT find AWS SDK")
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
  check_symbol_exists(_dyld_get_image_name "mach-o/dyld.h"
                      HAS_DYLD_GET_IMAGE_NAME)
else()
  set(CMAKE_REQUIRED_DEFINITIONS "-D_GNU_SOURCE=1")
  check_symbol_exists(dl_iterate_phdr "link.h" HAS_DL_ITERATE_PHDR)
endif()

set(mlxdata-src
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/Array.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/Sample.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/Dataset.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/Buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/Stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/BatchShape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/CSVReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/FileFetcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Graph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Numpy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/State.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/TARReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/ThreadController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/ThreadPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Tokenizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/BPETokenizer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Levenshtein.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Version.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/audio/Audio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/audio/AudioSndfile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/audio/AudioSampleRate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/image/ImageTransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/image/ImageIO.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/image/ImageSTBI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/image/ImageJPEG.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/video/Video.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/video/VideoFFMPEG.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Batch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/DynamicBatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/FromStream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/FromVector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/FilesFromTAR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Partition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Perm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Shuffle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/buffer/Transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Stream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Batch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Buffered.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/DynamicBatch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Compose.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/CSVReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/FromBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/LineReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/OrderedPrefetch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Partition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Prefetch.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Repeat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Shuffle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/SlidingWindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/stream/Transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Op.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/FilterByShape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/FilterKey.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/ReadFromTAR.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/KeyTransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/SampleTransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Shape.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Slice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/RenameKey.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/SaveImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/LoadAudio.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/LoadImage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/LoadFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/LoadNumpy.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/LoadVideo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Pad.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Shard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Squeeze.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Tokenize.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/ImageTransform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/RemoveValue.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mlx/data/op/Replace.cpp)

if(AWSSDK_FOUND)
  list(APPEND mlxdata-src
       ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/AWSFileFetcher.cpp)
endif()

add_library(mlxdata STATIC ${mlxdata-src})

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
target_link_libraries(mlxdata PRIVATE $<BUILD_INTERFACE:stb>
                                      $<BUILD_INTERFACE:bxzstr>)
target_include_directories(
  mlxdata PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                 $<INSTALL_INTERFACE:include>)
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

if(FFMPEG_avformat_FOUND)
  target_include_directories(mlxdata PRIVATE ${FFMPEG_INCLUDE_DIRS})
  target_link_libraries(mlxdata PRIVATE ${FFMPEG_LIBRARIES})
  target_compile_definitions(mlxdata PUBLIC MLX_HAS_FFMPEG)
endif()

# Version extra definitions
get_directory_property(BXZSTR_HAS_ZLIB DIRECTORY ${bxzstr_SOURCE_DIR}
                                                 DEFINITION ZLIB_FOUND)
get_directory_property(BXZSTR_HAS_BZIP2 DIRECTORY ${bxzstr_SOURCE_DIR}
                                                  DEFINITION BZIP2_FOUND)
get_directory_property(BXZSTR_HAS_LZMA DIRECTORY ${bxzstr_SOURCE_DIR}
                                                 DEFINITION LIBLZMA_FOUND)
get_directory_property(BXZSTR_HAS_ZSTD DIRECTORY ${bxzstr_SOURCE_DIR}
                                                 DEFINITION ZSTD_FOUND)
set(VERSION_DEFINITIONS "MLX_DATA_VERSION=${MLX_DATA_VERSION}")
if(BXZSTR_HAS_ZLIB)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_ZLIB")
endif()
if(BXZSTR_HAS_BZIP2)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_BZIP2")
endif()
if(BXZSTR_HAS_LZMA)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_LZMA")
endif()
if(BXZSTR_HAS_ZSTD)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_ZSTD")
endif()
if(SNDFILE_SUPPORTS_FLAC)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_FLAC")
endif()
if(SNDFILE_SUPPORTS_VORBIS OR SNDFILE_SUPPORTS_OPUS)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_OGG")
endif()
if(SNDFILE_SUPPORTS_VORBIS)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_VORBIS")
endif()
if(SNDFILE_SUPPORTS_OPUS)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_OPUS")
endif()
if(SNDFILE_SUPPORTS_MPEG)
  list(APPEND VERSION_DEFINITIONS "MLX_HAS_MPEG")
  if(MPG123_HAS_DISTVERSION)
    list(APPEND VERSION_DEFINITIONS "MLX_HAS_MPEG_DISTVERSION")
  endif()
endif()
set_property(
  SOURCE ${CMAKE_CURRENT_LIST_DIR}/mlx/data/core/Version.cpp
  APPEND
  PROPERTY COMPILE_DEFINITIONS ${VERSION_DEFINITIONS})

include("tests/CMakeLists.txt")

if(MLX_BUILD_PYTHON_BINDINGS)
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
  INCLUDES
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install headers
install(
  DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/mlx
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT headers
  FILES_MATCHING
  PATTERN "*.h")

# Install cmake config
set(MLX_DATA_CMAKE_BUILD_CONFIG ${CMAKE_BINARY_DIR}/MLXDataConfig.cmake)
set(MLX_DATA_CMAKE_BUILD_VERSION_CONFIG
    ${CMAKE_BINARY_DIR}/MLXDataConfigVersion.cmake)
set(MLX_DATA_CMAKE_INSTALL_MODULE_DIR share/cmake/MLXData)

install(
  EXPORT MLXDataTargets
  FILE MLXDataTargets.cmake
  DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR})

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  ${MLX_DATA_CMAKE_BUILD_VERSION_CONFIG}
  COMPATIBILITY SameMajorVersion
  VERSION ${MLX_DATA_VERSION})

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/mlx-data.pc.in ${MLX_DATA_CMAKE_BUILD_CONFIG}
  INSTALL_DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
  PATH_VARS CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
            MLX_DATA_CMAKE_INSTALL_MODULE_DIR)

install(FILES ${MLX_DATA_CMAKE_BUILD_CONFIG}
              ${MLX_DATA_CMAKE_BUILD_VERSION_CONFIG}
        DESTINATION ${MLX_DATA_CMAKE_INSTALL_MODULE_DIR})
