// Copyright © 2023 Apple Inc.

#include "mlx/data/core/audio/AudioPrivate.h"

namespace mlx {
namespace data {
namespace core {
namespace audio {

std::shared_ptr<Array> load(const std::string& path, AudioInfo* info) {
  return load_sndfile(path, info);
}

std::shared_ptr<Array> load(
    const std::shared_ptr<const Array>& contents,
    AudioInfo* info) {
  return load_sndfile(contents, info);
}

AudioInfo info(const std::string& path) {
  return info_sndfile(path);
}

AudioInfo info(const std::shared_ptr<const Array>& contents) {
  return info_sndfile(contents);
}

void verify_audio(const std::shared_ptr<const Array>& audio) {
  auto dimensions = audio->shape().size();
  if (dimensions != 2) {
    throw std::runtime_error(
        "verifyAudio: audio must be 2 dimension Array (SC)");
  }
}

} // namespace audio
} // namespace core
} // namespace data
} // namespace mlx
