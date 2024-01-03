cmake_minimum_required(VERSION 3.6)

project(mlx-data-deps)

include(ExternalProject)

set(PATH ${CMAKE_BINARY_DIR}/deps/bin:$ENV{PATH})
set(PKG_CONFIG_PATH
    ${CMAKE_BINARY_DIR}/deps/lib/pkgconfig:${CMAKE_BINARY_DIR}/deps/lib64/pkgconfig
)

# note: do not pass spaced-separated string to CMAKE_ARGS, as it will be
# interpreted as a single argument. Either no quote "string" or ; separated

# note: we use DOWNLOAD_EXTRACT_TIMESTAMP 1 as recent versions of cmake (>=
# 3.24) changed the previous behavior. This causes problems in some packages,
# where configure feels it is not up-to-date and try to run automake.

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
  set(X86 TRUE)
else()
  set(X86 FALSE)
endif()

ExternalProject_Add(
  pkg-config
  URL http://pkgconfig.freedesktop.org/releases/pkg-config-0.29.2.tar.gz
      http://fresh-center.net/linux/misc/pkg-config-0.29.2.tar.gz
  CONFIGURE_COMMAND ./configure --prefix=${CMAKE_BINARY_DIR}/deps
                    --disable-shared --with-internal-glib
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  libiconv
  URL https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.17.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

if(APPLE)
  set(GETTEXT_EXTRA_FLAGS --with-included-gettext)
else()
  set(GETTEXT_EXTRA_FLAGS)
endif()

ExternalProject_Add(
  gettext
  DEPENDS libiconv
  URL https://ftp.gnu.org/gnu/gettext/gettext-0.22.4.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared --with-included-glib
    --with-included-libcroco --with-included-libunistring --with-included-libxml
    --disable-java --disable-csharp --without-git --without-cvs --without-xz
    ${GETTEXT_EXTRA_FLAGS}
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  zlib
  URL https://www.zlib.net/zlib-1.3.tar.gz
  CONFIGURE_COMMAND PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC
                    ./configure --prefix=${CMAKE_BINARY_DIR}/deps --static
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# patch bzip2 for -fPIC compilation
ExternalProject_Add(
  bzip2
  URL https://sourceware.org/pub/bzip2/bzip2-1.0.8.tar.gz
  PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/cmake/bzip2-1.0.8.patch
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND make install PREFIX=${CMAKE_BINARY_DIR}/deps
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# lzma support
ExternalProject_Add(
  xz
  URL https://downloads.sourceforge.net/project/lzmautils/xz-5.4.5.tar.gz
      https://tukaani.org/xz/xz-5.4.5.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# needed on x86 by few projects
ExternalProject_Add(
  nasm
  URL https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/nasm-2.16.01.tar.xz
  CONFIGURE_COMMAND PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} ./configure
                    --prefix=${CMAKE_BINARY_DIR}/deps
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  libjpeg-turbo
  DEPENDS nasm
  URL https://downloads.sourceforge.net/project/libjpeg-turbo/3.0.0/libjpeg-turbo-3.0.0.tar.gz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DENABLE_SHARED=OFF
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_ASM_NASM_COMPILER=${CMAKE_BINARY_DIR}/deps/bin/nasm
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  libsamplerate
  URL https://github.com/libsndfile/libsamplerate/archive/refs/tags/0.2.2.tar.gz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DBUILD_SHARED_LIBS=OFF
             -DLIBSAMPLERATE_EXAMPLES=OFF
             -DBUILD_TESTING=OFF
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# note: cmake version fails to install
ExternalProject_Add(
  libogg
  URL https://ftp.osuosl.org/pub/xiph/releases/ogg/libogg-1.3.5.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  flac
  DEPENDS libogg pkg-config gettext
  URL https://github.com/xiph/flac/releases/download/1.4.3/flac-1.4.3.tar.xz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DINSTALL_MANPAGES=OFF
             -DBUILD_TESTING=OFF
             -DBUILD_PROGRAMS=OFF
             -DBUILD_EXAMPLES=OFF
             -DBUILD_DOCS=OFF
             -DBUILD_SHARED_LIBS=OFF
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  libvorbis
  DEPENDS libogg pkg-config
  URL https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.xz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DBUILD_SHARED_LIBS=OFF
             -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/deps
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  opus
  URL https://github.com/xiph/opus/releases/download/v1.4/opus-1.4.tar.gz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DOPUS_BUILD_SHARED_LIBRARY=OFF
             -DOPUS_BUILD_TESTING=OFF
             -DOPUS_BUILD_PROGRAMS=OFF
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  lame
  URL https://downloads.sourceforge.net/project/lame/lame/3.100/lame-3.100.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-debug --disable-frontend
    --disable-shared --disable-gtktest
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

set(MPG123_COMPILE_OPTIONS "")
if(APPLE)
  set(MPG123_COMPILE_OPTIONS "--with-default-audio=coreaudio"
                             ${MPG123_COMPILE_OPTIONS})
endif()
if(X86)
  set(MPG123_COMPILE_OPTIONS "--with-cpu=x86-64" ${MPG123_COMPILE_OPTIONS})
else()
  set(MPG123_COMPILE_OPTIONS "--with-cpu=aarch64" ${MPG123_COMPILE_OPTIONS})
endif()

ExternalProject_Add(
  mpg123
  URL https://sourceforge.net/projects/mpg123/files/mpg123/1.32.3/mpg123-1.32.3.tar.bz2
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared --disable-components
    --enable-libmpg123 ${MPG123_COMPILE_OPTIONS}
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  libsndfile
  DEPENDS flac
          lame
          libogg
          libvorbis
          mpg123
          opus
          pkg-config
  URL https://github.com/libsndfile/libsndfile/releases/download/1.2.2/libsndfile-1.2.2.tar.xz
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DOPUS_BUILD_SHARED_LIBRARY=OFF
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  openssl
  URL https://www.openssl.org/source/openssl-1.1.1q.tar.gz
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./config
    no-shared no-idea no-mdc2 no-rc5 no-tests --prefix=${CMAKE_BINARY_DIR}/deps
  BUILD_COMMAND make depend && make
  INSTALL_COMMAND make install_sw
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  curl
  URL https://curl.se/download/curl-8.5.0.tar.bz2
  DEPENDS openssl pkg-config
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} CFLAGS=-fPIC ./configure
    --with-openssl --disable-shared --prefix=${CMAKE_BINARY_DIR}/deps
  BUILD_COMMAND make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# patch aws-sdk-cpp to add necessary static libs at link time
ExternalProject_Add(
  aws-s3
  GIT_REPOSITORY https://github.com/aws/aws-sdk-cpp.git
  GIT_TAG 1.11.231
  GIT_SHALLOW 1
  PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/cmake/aws-1.11.231.patch
  DEPENDS openssl curl pkg-config
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
             -DBUILD_ONLY=s3
             -DBUILD_SHARED_LIBS=OFF
             -DBUILD_TESTING=OFF
             -DAUTORUN_UNIT_TESTS=OFF
             -DENABLE_TESTING=OFF
             -DCMAKE_POSITION_INDEPENDENT_CODE=ON
             -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/deps
  INSTALL_DIR ${CMAKE_BINARY_DIR}/deps
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# patch xvidcore to disable shared lib we build single thread, otherwise it may
# fail sometimes
ExternalProject_Add(
  xvidcore
  DEPENDS nasm
  URL https://downloads.xvid.com/downloads/xvidcore-1.3.7.tar.bz2
  PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/cmake/xvidcore-1.3.7.patch
  CONFIGURE_COMMAND
    cd build/generic && PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH}
    CFLAGS=-fPIC ./configure --prefix=${CMAKE_BINARY_DIR}/deps
  BUILD_COMMAND cd build/generic && make -j1
  INSTALL_COMMAND cd build/generic && make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

ExternalProject_Add(
  ffmpeg
  DEPENDS nasm
          zlib
          lame
          libogg
          opus
          libvorbis
          xvidcore
          pkg-config
  URL https://ffmpeg.org/releases/ffmpeg-6.1.1.tar.bz2
  CONFIGURE_COMMAND
    PATH=${PATH} PKG_CONFIG_PATH=${PKG_CONFIG_PATH} ./configure
    --prefix=${CMAKE_BINARY_DIR}/deps --disable-shared --enable-pic
    --enable-runtime-cpudetect --enable-libvorbis --enable-libopus
    --disable-iconv --pkg-config=${CMAKE_BINARY_DIR}/deps/bin/pkg-config
    --extra-ldflags=-L${CMAKE_BINARY_DIR}/deps/lib\ -L${CMAKE_BINARY_DIR}/deps/lib64
    --extra-libs=-lvorbis\ -logg\ -lm
  BUILD_COMMAND PATH=${CMAKE_BINARY_DIR}/deps/bin:$ENV{PATH} && make
  INSTALL_COMMAND make install
  BUILD_IN_SOURCE 1
  DOWNLOAD_EXTRACT_TIMESTAMP 1)

# check -Bsymbolic works (this is for ffmpeg static link)
include(CheckCSourceCompiles)
list(APPEND CMAKE_REQUIRED_LIBRARIES "-Wl,-Bsymbolic")
check_c_source_compiles(
  [=[
 	int main ()
 	{
 	  return 0;
 	}
  ]=]
  HAS_BSYMBOLIC)
list(REMOVE_ITEM CMAKE_REQUIRED_LIBRARIES "-Wl,-Bsymbolic")
set(MLX_DATA_MODULE_LINKER_FLAGS)
if(HAS_BSYMBOLIC)
  set(MLX_DATA_MODULE_LINKER_FLAGS -Wl,-Bsymbolic)
endif()

# CMake does not like when source dir depends on targets which are in a
# subdirectory of the top source dir. We thus make a symlink in the build dir to
# avoid any error message (CMake also create the directory mlx-data beforehand,
# expecting it to be populated -- we remove this directory first in that
# respect)
ExternalProject_Add(
  mlx-data
  DOWNLOAD_COMMAND
    ${CMAKE_COMMAND} -E remove_directory mlx-data && ${CMAKE_COMMAND} -E
    create_symlink ${CMAKE_SOURCE_DIR}/.. mlx-data
  UPDATE_COMMAND ""
  DEPENDS zlib
          bzip2
          xz
          libsndfile
          libsamplerate
          libjpeg-turbo
          ffmpeg
          aws-s3
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
             -DCMAKE_PREFIX_PATH=${CMAKE_BINARY_DIR}/deps
             -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
             -DMLX_BUILD_PYTHON_BINDINGS=${MLX_BUILD_PYTHON_BINDINGS}
             -DMLX_DATA_VERSION=${MLX_DATA_VERSION}
             -DCMAKE_MODULE_LINKER_FLAGS=${MLX_DATA_MODULE_LINKER_FLAGS})
