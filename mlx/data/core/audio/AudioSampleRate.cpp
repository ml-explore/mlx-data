// Copyright © 2023 Apple Inc.

#include <cmath>
#include "mlx/data/core/audio/Audio.h"

#ifdef MLX_HAS_SAMPLERATE
#include <samplerate.h>
#endif

namespace mlx {
namespace data {
namespace core {
namespace audio {

#ifdef MLX_HAS_SAMPLERATE

std::shared_ptr<Array> resample(
    const std::shared_ptr<Array>& audio,
    ResampleMode resample_mode,
    int src_sample_rate,
    int dst_sample_rate) {
  if ((dst_sample_rate <= 0) || (src_sample_rate == dst_sample_rate)) {
    return audio;
  }

  int64_t audio_channels = channels(audio);
  int64_t audio_length = frames(audio);

  double length_scale = static_cast<double>(dst_sample_rate) /
      static_cast<double>(src_sample_rate);
  int64_t new_audio_length =
      static_cast<int64_t>(std::floor(audio_length * length_scale));
  auto result = std::make_shared<Array>(
      ArrayType::Float, new_audio_length, audio_channels);
  SRC_DATA src_data;
  src_data.data_in = audio->data<float>();
  src_data.input_frames = audio_length;
  src_data.data_out = result->data<float>();
  src_data.output_frames = new_audio_length;
  src_data.src_ratio = length_scale;
  auto status = src_simple(&src_data, (int)resample_mode, audio_channels);
  if (status) {
    std::string msg("audio: libsamplerate failed with: ");
    msg += std::string(src_strerror(status));
    throw std::runtime_error(msg);
  }

  if (new_audio_length != src_data.output_frames_gen) {
    std::vector<int64_t> offset(2, 0);
    auto new_shape = result->shape();
    new_shape[0] = src_data.output_frames_gen;
    result = array::sub(result, offset, new_shape);
  }

  return result;
}

#else

std::shared_ptr<Array> resample(
    const std::shared_ptr<Array>& audio,
    ResampleMode resample_mode,
    int src_sample_rate,
    int dst_sample_rate) {
  throw std::runtime_error(
      "audio: mlx was not compiled with sample rate conversion support (libsamplerate)");
}

#endif

} // namespace audio
} // namespace core
} // namespace data
} // namespace mlx
