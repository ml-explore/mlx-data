# bxzstr — A C++11 ZLib / libBZ2 / libLZMA / libZstd wrapper

Header-only library for using standard c++ iostreams to access streams
compressed with ZLib, libBZ2, libLZMA, or libZstd (.gz, .bz2, .xz, and
.zst files).

For decompression, the format is automatically detected. For
compression, the only parameter exposed is the compression algorithm.

bxzstr is a fork of the [zstr](https://github.com/mateidavid/zstr)
library by [Matei David](https://github.com/mateidavid), and the core
functionality of this library remains largely the same.

## Input detection

The library automatically detects whether the input stream is
compressed or not, and with which algorithm. The detection is based on
identifying the headers based on the magic numbers:
* GZip header, starting with **1F 8B**
* ZLib header, starting with **78 01**, **78 9c**, and **78 DA**
* BZ2 header, starting with **42 5a 68**
* LZMA header, starting with **FD 37 7A 58 5A 00**
* ZSTD header, starting with **28 B5 2F FD**

when no header is identified, the stream is treated as plain text (uncompressed).

## Usage
The streams can be accessed through 6 classes that function similarly
to their standard library counterparts

* `bxz::istreambuf` is the core decompression class.
* `bxz::ostreambuf` is the core compression class.
* `bxz::istream` is a wrapper for the `bxz::istreambuf` class.
* `bxz::ostream` is a wrapper for the `bxz::ostreambuf` class.
* `bxz::ifstream` is a wrapper for the `bxz::istreambuf` class that
  can be used to open a file and read decompressed data from it.
* `bxz::ofstream` is a wrapper for the `bxz::ostreambuf` class that
  can be used to write compressed data to a file.

For the classes derived from `bxz::ostreambuf`, the compression
algorithm must be specified as the second argument:
```
bxz::ofstream("filename", bxz::z);
bxz::ostream(std::cin, bxz::bz2);
bxz::ostreambuf(std::cin.rdbuf(), bxz::lzma);
```

It's also possible to specify the compression level (1-9) as the third
parameter (default level is 6):
```
bxz::ofstream("filename", bxz::z, 1);
bxz::ostream(std::cin, bxz::bz2, 5);
bxz::ostreambuf(std::cin.rdbuf(), bxz::lzma, 9);
```

If the stream objects fail at any point, `failbit` exception mask will
be turned on.

## Configuration
You can use the library without one of libz, libbz2, or liblzma by
modifying the `config.hpp` file. For example, to disable lzma support,
set BXZSTR_LZMA_SUPPORT to 0:
```
#ifndef BXZSTR_CONFIG_HPP
#define BXZSTR_CONFIG_HPP

#define BXZSTR_Z_SUPPORT 1
#define BXZSTR_BZ2_SUPPORT 1
#define BXZSTR_LZMA_SUPPORT 0
#define BXZSTR_ZSTD_SUPPORT 0

#endif
```
The rest of the project will adapt accordingly.

## Automatic configuration
It is also possible to configure the header automatically with CMake,
e. g. as part of a larger project, by running
```
cmake .
```
in the root directory. CMake will modify `config.hpp` to match the
libraries supported on the system. If the
[find_package](https://cmake.org/cmake/help/v3.0/command/find_package.html)
command has already been run in CMake, automatic configuration will
respect the results instead of running find_package again.

## Testing
bxzstr implements (non-exhaustive) testing for parts of the source
code using the [googletest](https://github.com/google/googletest)
framework. For more details, see the documentation at
[docs/development/building_tests.md](/docs/development/building_tests.md).

## Requirements and dependencies
* Compiler with c++11 support
* CMake v3.0 or greater (for automatic config)
* libz, libbz2, liblzma, and/or libzstd

## License
The source code from this project is subject to the terms of the
Mozilla Public License, v. 2.0. A copy of the MPL is supplied with the
project, or can be obtained at
[https://mozilla.org/MPL/2.0/](https://mozilla.org/MPL/2.0/).
