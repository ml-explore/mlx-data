// Copyright © 2023-2025 Apple Inc.

#include "mlx/data/core/Version.h"
#include <cstring>

extern "C" {
#if MLX_HAS_ZLIB
#include <zlib.h>
#endif

#if MLX_HAS_BZIP2
#include <bzlib.h>
#endif

#if MLX_HAS_LZMA
#include <lzma.h>
#endif

#if MLX_HAS_ZSTD
#include <zstd.h>
#endif

#if MLX_HAS_SNDFILE
#include <sndfile.h>
// Support for old libsndfile versions
#if !defined(SF_FORMAT_MPEG)
#define SF_FORMAT_MPEG 0x230000
#endif
#if !defined(SF_FORMAT_MPEG_LAYER_III)
#define SF_FORMAT_MPEG_LAYER_I 0x0082
#endif
#if !defined(SF_FORMAT_OPUS)
#define SF_FORMAT_OPUS 0x0064
#endif
// FLAC, Vorbis, Opus, mp123 are not
// directly used from mlx-data so headers
// might not be there.
extern const char* FLAC__VERSION_STRING;
extern const char* vorbis_version_string(void);
extern const char* opus_get_version_string(void);
extern const char* mpg123_distversion(
    unsigned int* major,
    unsigned int* minor,
    unsigned int* patch);
#endif

#if MLX_HAS_SAMPLERATE
#include <samplerate.h>
#endif

#if MLX_HAS_JPEG
#include <jerror.h>
#include <jpeglib.h>
#endif

#if MLX_HAS_FFMPEG
#include <libavutil/avutil.h>
#endif
}

#if MLX_HAS_AWS
#include <aws/core/Version.h>
#endif

namespace {
std::string zlib_get_version() {
#if MLX_HAS_ZLIB
  return std::string(zlibVersion());
#else
  return {};
#endif
}
std::string bzip2_get_version() {
#if MLX_HAS_BZIP2
  return std::string(BZ2_bzlibVersion());
#else
  return {};
#endif
}
std::string lzma_get_version() {
#if MLX_HAS_LZMA
  return std::string(lzma_version_string());
#else
  return {};
#endif
}
std::string zstd_get_version() {
#if MLX_HAS_ZSTD
  return std::string(ZSTD_versionString());
#else
  return {};
#endif
}
#if MLX_HAS_JPEG
void jpeg_error_custom_noop(j_common_ptr cinfo) {}
std::string jpeg_get_version() {
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr error_mgr;
  error_mgr.error_exit = &jpeg_error_custom_noop;
  cinfo.err = &error_mgr;
  jpeg_CreateDecompress(&cinfo, -1 /*version*/, sizeof(cinfo));
  return std::string(std::to_string(cinfo.err->msg_parm.i[0]));
}
#else
std::string jpeg_get_version() {
  return {};
}
#endif
#if MLX_HAS_SNDFILE
std::string sndfile_get_version() {
  return std::string(sf_version_string());
}
sf_count_t noop_vio_get_filelen(void* user_data) {
  return 0;
}
sf_count_t noop_vio_seek(sf_count_t offset, int whence, void* user_data) {
  return 0;
}
sf_count_t noop_vio_read(void* ptr, sf_count_t count, void* user_data) {
  return count;
}
sf_count_t noop_vio_write(const void* ptr, sf_count_t count, void* user_data) {
  return count;
}
sf_count_t noop_vio_tell(void* user_data) {
  return 0;
}
bool sndfile_supports_format(int format) {
  SF_VIRTUAL_IO noop_io = {
      noop_vio_get_filelen,
      noop_vio_seek,
      noop_vio_read,
      noop_vio_write,
      noop_vio_tell};
  SF_INFO info;
  info.samplerate = 16000;
  info.channels = 2;
  info.format = format;
  return sf_open_virtual(&noop_io, SFM_WRITE, &info, NULL) != nullptr;
}
std::string flac_get_version() {
  if (sndfile_supports_format(SF_FORMAT_FLAC | SF_FORMAT_PCM_16)) {
#if MLX_HAS_FLAC
    return std::string(FLAC__VERSION_STRING);
#else
    return "unknown";
#endif
  } else {
    return {};
  }
}
std::string vorbis_get_version() {
  if (sndfile_supports_format(SF_FORMAT_OGG | SF_FORMAT_VORBIS)) {
#if MLX_HAS_VORBIS
    return std::string(vorbis_version_string());
#else
    return "unknown";
#endif
  } else {
    return {};
  }
}
std::string opus_get_version() {
  if (sndfile_supports_format(SF_FORMAT_OGG | SF_FORMAT_OPUS)) {
#if MLX_HAS_OPUS
    return std::string(opus_get_version_string());
#else
    return "unknown";
#endif
  } else {
    return {};
  }
}
std::string mpg123_get_version() {
  if (sndfile_supports_format(SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_I)) {
#if MLX_HAS_MPEG
#if MLX_HAS_MPEG_DISTVERSION
    return std::string(mpg123_distversion(nullptr, nullptr, nullptr));
#else
    return std::string("unknown");
#endif
#else
    return "unknown";
#endif
  } else {
    return {};
  }
}
#else
std::string sndfile_get_version() {
  return {};
}
std::string flac_get_version() {
  return {};
}
std::string vorbis_get_version() {
  return {};
}
std::string opus_get_version() {
  return {};
}
std::string mpg123_get_version() {
  return {};
}
#endif
std::string samplerate_get_version() {
#if MLX_HAS_SAMPLERATE
  return std::string(src_get_version());
#else
  return {};
#endif
}
std::string ffmpeg_get_version() {
#if MLX_HAS_FFMPEG
  return std::string(av_version_info());
#else
  return {};
#endif
}
std::string aws_get_version() {
#if MLX_HAS_AWS
  return std::string(Aws::Version::GetVersionString());
#else
  return {};
#endif
}
} // namespace

#define MLX_DATA_VERSION_STRINGIFY_(x) #x
#define MLX_DATA_VERSION_STRINGIFY(x) MLX_DATA_VERSION_STRINGIFY_(x)

namespace {}

namespace mlx {
namespace data {
namespace core {

std::string version() {
  return std::string(MLX_DATA_VERSION_STRINGIFY(MLX_DATA_VERSION));
}

std::unordered_map<std::string, std::string> libs_version() {
  static std::unordered_map<std::string, std::string> libs;
  if (libs.empty()) {
    libs["zlib"] = zlib_get_version();
    libs["bzip2"] = bzip2_get_version();
    libs["lzma"] = lzma_get_version();
    libs["zstd"] = zstd_get_version();
    libs["jpeg"] = jpeg_get_version();
    libs["sndfile"] = sndfile_get_version();
    libs["sndfile_flac"] = flac_get_version();
    libs["sndfile_vorbis"] = vorbis_get_version();
    libs["sndfile_opus"] = opus_get_version();
    libs["sndfile_mpeg"] = mpg123_get_version();
    libs["samplerate"] = samplerate_get_version();
    libs["ffmpeg"] = ffmpeg_get_version();
    libs["aws"] = aws_get_version();
  }
  return libs;
}
} // namespace core
} // namespace data
} // namespace mlx
