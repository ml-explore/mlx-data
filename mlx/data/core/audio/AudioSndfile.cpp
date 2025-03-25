// Copyright © 2023 Apple Inc.

#include "mlx/data/core/audio/AudioPrivate.h"

#ifdef MLX_HAS_SNDFILE
#include <sndfile.hh>

extern "C" {

struct SfVioRo {
  void* data;
  sf_count_t size;
  sf_count_t pos;
};

static sf_count_t sf_vio_ro_get_filelen(void* user_data) {
  auto ctx = reinterpret_cast<SfVioRo*>(user_data);
  return ctx->size;
}

static sf_count_t
sf_vio_ro_seek(sf_count_t offset, int whence, void* user_data) {
  auto ctx = reinterpret_cast<SfVioRo*>(user_data);
  switch (whence) {
    case SEEK_CUR:
      ctx->pos += offset;
      break;
    case SEEK_SET:
      ctx->pos = offset;
      break;
    case SEEK_END:
      ctx->pos = ctx->size + offset;
      break;
    default:
      throw std::invalid_argument(
          "LoadAudio: internal error: whence is invalid");
  }
  if (ctx->pos < 0 || ctx->pos > ctx->size) {
    throw std::invalid_argument(
        "LoadAudio: internal error: position is invalid");
  }
  return ctx->pos;
}

static sf_count_t sf_vio_ro_read(void* ptr, sf_count_t count, void* user_data) {
  auto ctx = reinterpret_cast<SfVioRo*>(user_data);
  if (count > ctx->size - ctx->pos) {
    count = ctx->size - ctx->pos;
  }
  if (count > 0) {
    std::memcpy(ptr, (char*)ctx->data + ctx->pos, count);
    ctx->pos += count;
  }
  return count;
}

static sf_count_t sf_vio_ro_write(
    const void* /* ptr */,
    sf_count_t /* count */,
    void* /* user_data */) {
  throw std::invalid_argument("LoadAudio: internal error: read-only stream");
  return 0;
}

static sf_count_t sf_vio_ro_tell(void* user_data) {
  auto ctx = reinterpret_cast<SfVioRo*>(user_data);
  return ctx->pos;
}

static SF_VIRTUAL_IO vsf = {
    sf_vio_ro_get_filelen,
    sf_vio_ro_seek,
    sf_vio_ro_read,
    sf_vio_ro_write,
    sf_vio_ro_tell};
}
#endif

namespace mlx {
namespace data {
namespace core {
namespace audio {

#ifdef MLX_HAS_SNDFILE

static void sf_check_error(
    const SndfileHandle& sf,
    bool from_memory,
    const std::string& path) {
  if (sf.error()) {
    std::string msg("LoadAudio: libsndfile failed with: ");
    msg += std::string(sf.strError());
    if (!from_memory) {
      msg += " (" + path + ")";
    }
    throw std::runtime_error(msg);
  }
}

static AudioInfo read_info(SndfileHandle& sf) {
  AudioInfo info;

  info.frames = sf.frames();
  info.channels = sf.channels();
  info.sampleRate = sf.samplerate();

  return info;
}

static std::shared_ptr<Array> read_data(SndfileHandle& sf, AudioInfo info) {
  std::shared_ptr<Array> dst;

  dst = std::make_shared<Array>(ArrayType::Float, info.frames, info.channels);
  sf.readf(reinterpret_cast<float*>(dst->data()), info.frames);

  return dst;
}

std::shared_ptr<Array> load_sndfile(const std::string& path, AudioInfo* info) {
  SndfileHandle sf = SndfileHandle(path);
  sf_check_error(sf, false, path);

  AudioInfo audio_info = read_info(sf);
  if (info != nullptr) {
    *info = audio_info;
  }

  auto result = read_data(sf, audio_info);

  sf_check_error(sf, false, path);
  return result;
}

std::shared_ptr<Array> load_sndfile(
    const std::shared_ptr<Array>& contents,
    AudioInfo* info) {
  SfVioRo sfctx;
  sfctx.data = contents->data();
  sfctx.size = contents->size() * contents->itemsize();
  sfctx.pos = 0;
  SndfileHandle sf = SndfileHandle(vsf, &sfctx);
  sf_check_error(sf, true, "");

  AudioInfo audio_info = read_info(sf);
  if (info != nullptr) {
    *info = audio_info;
  }

  auto result = read_data(sf, audio_info);

  sf_check_error(sf, true, "");
  return result;
}

AudioInfo info_sndfile(const std::string& path) {
  SndfileHandle sf = SndfileHandle(path);
  sf_check_error(sf, false, path);

  AudioInfo audio_info = read_info(sf);
  return audio_info;
}

AudioInfo info_sndfile(const std::shared_ptr<Array>& contents) {
  SfVioRo sfctx;
  sfctx.data = contents->data();
  sfctx.size = contents->size() * contents->itemsize();
  sfctx.pos = 0;
  SndfileHandle sf = SndfileHandle(vsf, &sfctx);
  sf_check_error(sf, true, "");

  AudioInfo audio_info = read_info(sf);
  return audio_info;
}

#else

volatile static void no_sndfile() {
  throw std::runtime_error(
      "audio: mlx was not compiled with audio support (libsndfile)");
}

std::shared_ptr<Array> load_sndfile(const std::string& path, AudioInfo* info) {
  no_sndfile();
}

std::shared_ptr<Array> load_sndfile(
    const std::shared_ptr<Array>& contents,
    AudioInfo* info) {
  no_sndfile();
}

AudioInfo info_sndfile(const std::string& path) {
  no_sndfile();
}

AudioInfo info_sndfile(const std::shared_ptr<Array>& contents) {
  no_sndfile();
}

#endif

} // namespace audio
} // namespace core
} // namespace data
} // namespace mlx
