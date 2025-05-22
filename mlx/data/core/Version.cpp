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
#if !defined(SF_FORMAT_MPEG_LAYER_I)
#define SF_FORMAT_MPEG_LAYER_I 0x0080
#endif
#if !defined(SF_FORMAT_OPUS)
#define SF_FORMAT_OPUS 0x0064
#endif
#endif

#if MLX_HAS_FLAC
#include <FLAC/format.h>
#endif

#if MLX_HAS_VORBIS
#include <vorbis/codec.h>
#endif

#if MLX_HAS_OPUS
#include <opus/opus.h>
#endif

#if MLX_HAS_MPEG
#include <mpg123.h>
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
bool sndfile_supports_format(int format) {
  SF_INFO sfinfo;
  memset(&sfinfo, 0, sizeof(sfinfo));
  sfinfo.samplerate = 44000;
  sfinfo.channels = 1;
  sfinfo.format = format;
  return sf_format_check(&sfinfo);
}
#else
std::string sndfile_get_version() {
  return {};
}
#endif
std::string flac_get_version() {
#if MLX_HAS_FLAC
  return std::string(FLAC__VERSION_STRING);
#else
  return {};
#endif
}
std::string vorbis_get_version() {
#if MLX_HAS_VORBIS
  return std::string(vorbis_version_string());
#else
  return {};
#endif
}
std::string opus_get_version() {
#if MLX_HAS_OPUS
  return std::string(opus_get_version_string());
#else
  return {};
#endif
}
std::string mpg123_get_version() {
#if MLX_HAS_MPEG
  return std::string(mpg123_distversion(nullptr, nullptr, nullptr));
#else
  return {};
#endif
}
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

#define STRING_MLX_DATA_VERSION_(x) #x
#define STRING_MLX_DATA_VERSION(x) STRING_MLX_DATA_VERSION_(x)

namespace {}

namespace mlx {
namespace data {
namespace core {

std::string version() {
  return std::string(STRING_MLX_DATA_VERSION_(MLX_DATA_VERSION));
}

std::unordered_map<std::string, bool> supported_libs() {
  static std::unordered_map<std::string, bool> libs;
  if (libs.empty()) {
#if MLX_HAS_ZLIB
    libs["zlib"] = true;
#else
    libs["zlib"] = false;
#endif
#if MLX_HAS_BZIP2
    libs["bzip2"] = true;
#else
    libs["bzip2"] = false;
#endif
#if MLX_HAS_LIBZMA
    libs["libzma"] = true;
#else
    libs["libzma"] = false;
#endif
#if MLX_HAS_ZSTD
    libs["zstd"] = true;
#else
    libs["zstd"] = false;
#endif
#if MLX_HAS_JPEG
    libs["jpeg"] = true;
#else
    libs["jpeg"] = false;
#endif
#if MLX_HAS_SNDFILE
// Support for old libsndfile versions
#if !defined(SF_FORMAT_MPEG)
#define SF_FORMAT_MPEG 0x230000
#endif
#if !defined(SF_FORMAT_MPEG_LAYER_I)
#define SF_FORMAT_MPEG_LAYER_I 0x0080
#endif
#if !defined(SF_FORMAT_OPUS)
#define SF_FORMAT_OPUS 0x0064
#endif
    libs["sndfile"] = true;
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(sfinfo));
    sfinfo.samplerate = 44000;
    sfinfo.channels = 1;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    libs["sndfile_wav"] = sf_format_check(&sfinfo);
    sfinfo.format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
    libs["sndfile_flac"] = sf_format_check(&sfinfo);
    sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
    libs["sndfile_ogg"] = sf_format_check(&sfinfo);
    sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_OPUS;
    libs["sndfile_opus"] = sf_format_check(&sfinfo);
    sfinfo.format = SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_I;
    libs["sndfile_mpeg"] = sf_format_check(&sfinfo);
#else
    libs["sndfile"] = false;
    libs["sndfile_wav"] = false;
    libs["sndfile_flac"] = false;
    libs["sndfile_ogg"] = false;
    libs["sndfile_opus"] = false;
    libs["sndfile_mpeg"] = false;
#endif
#if MLX_HAS_AWS
    libs["aws"] = true;
#else
    libs["aws"] = false;
#endif
#if MLX_HAS_FFMPEG
    libs["ffmpeg"] = true;
#else
    libs["ffmpeg"] = false;
#endif
  }
  return libs;
}
std::unordered_map<std::string, std::string> supported_libs_version() {
  static std::unordered_map<std::string, std::string> libs;
  if (libs.empty()) {
    libs["zlib"] = zlib_get_version();
    libs["bzip2"] = bzip2_get_version();
    libs["lzma"] = lzma_get_version();
    libs["zstd"] = zstd_get_version();
    libs["jpeg"] = jpeg_get_version();
    libs["sndfile"] = sndfile_get_version();
    libs["sndfile_flac"] =
        (sndfile_supports_format(SF_FORMAT_FLAC | SF_FORMAT_PCM_16) ? "enabled"
                                                                    : "");
    libs["sndfile_vorbis"] =
        (sndfile_supports_format(SF_FORMAT_OGG | SF_FORMAT_VORBIS) ? "enabled"
                                                                   : "");
    libs["sndfile_opus"] =
        (sndfile_supports_format(SF_FORMAT_OGG | SF_FORMAT_OPUS) ? "enabled"
                                                                 : "");
    libs["sndfile_mpeg"] =
        (sndfile_supports_format(SF_FORMAT_MPEG | SF_FORMAT_MPEG_LAYER_I)
             ? "enabled"
             : "");
    libs["flac"] = flac_get_version();
    libs["vorbis"] = vorbis_get_version();
    libs["opus"] = opus_get_version();
    libs["mpg123"] = mpg123_get_version();
    libs["samplerate"] = samplerate_get_version();
    libs["ffmpeg"] = ffmpeg_get_version();
    libs["aws"] = aws_get_version();
  }
  return libs;
}
} // namespace core
} // namespace data
} // namespace mlx
