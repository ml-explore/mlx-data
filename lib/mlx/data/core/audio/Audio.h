// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Array.h"

namespace mlx {
namespace data {
namespace core {
namespace audio {

/// Image metadata
struct AudioInfo {
 public:
  int64_t frames;
  int sampleRate;
  int channels;
};

std::shared_ptr<Array> load(const std::string& path, AudioInfo* info);
std::shared_ptr<Array> load(
    const std::shared_ptr<const Array>& contents,
    AudioInfo* info);

AudioInfo info(const std::string& path);
AudioInfo info(const std::shared_ptr<const Array>& contents);

/// Verify that the given Array is structured like audio:
/// two dimensions (s, c).
void verify_audio(const std::shared_ptr<const Array>& audio);

/// Return the number of frames in the audio.  Requires that `verifyAudio()` be
/// called previously.
inline const int64_t frames(const std::shared_ptr<const Array>& audio) {
  return audio->shape()[0];
}

/// Return the channel count of the audio.  Requires that `verifyAudio()` be
/// called previously.
inline const int64_t channels(const std::shared_ptr<const Array>& audio) {
  return audio->shape()[1];
}

/// Resample mode -- these should match 1:1 with the libsamplerate enum, e.g.
/// SRC_SINC_BEST_QUALITY
enum class ResampleMode {
  best = 0,
  medium = 1,
  fastest = 2,
  zeroOrderHold = 3,
  linear = 4,
};

std::shared_ptr<Array> resample(
    const std::shared_ptr<const Array>& audio,
    ResampleMode resample_mode,
    int src_sample_rate,
    int dst_sample_rate);

} // namespace audio
} // namespace core
} // namespace data
} // namespace mlx
