#include <cmath>
#include <filesystem>
#include <unordered_map>

#include "mlx/data/core/audio/Audio.h"
#include "mlx/data/op/LoadAudio.h"

namespace mlx {
namespace data {
namespace op {

LoadAudio::LoadAudio(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool from_memory,
    LoadAudioInfo info_type,
    int sample_rate,
    LoadAudioResamplingQuality resampling_quality,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      prefix_(prefix),
      info_(info),
      from_memory_(from_memory),
      infoType_(info_type),
      sampleRate_(sample_rate),
      resamplingQuality_(resampling_quality) {}

std::shared_ptr<Array> LoadAudio::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::filesystem::path path;
  if (!from_memory_) {
    if (src->type() != ArrayType::Int8) {
      throw std::runtime_error("LoadAudio: expected filename (int8 array)");
    }
    path = prefix_;
    std::string filename(reinterpret_cast<char*>(src->data()), src->size());
    path /= filename;
  }

  std::shared_ptr<Array> dst;

  if (info_) {
    auto audio_info =
        from_memory_ ? core::audio::info(src) : core::audio::info(path);

    auto audio_length = audio_info.frames;
    auto audio_channels = audio_info.channels;
    auto audio_sample_rate = audio_info.sampleRate;
    if (infoType_ == LoadAudioInfo::All) {
      std::vector<int64_t> info(
          {audio_length, audio_channels, audio_sample_rate});
      dst = std::make_shared<Array>(info);
    } else if (infoType_ == LoadAudioInfo::NumFrames) {
      dst = std::make_shared<Array>(audio_length);
    } else if (infoType_ == LoadAudioInfo::NumChannels) {
      dst = std::make_shared<Array>(audio_channels);
    } else if (infoType_ == LoadAudioInfo::SampleRate) {
      dst = std::make_shared<Array>(audio_sample_rate);
    } else if (infoType_ == LoadAudioInfo::NumSeconds) {
      dst = std::make_shared<Array>(
          static_cast<double>(audio_length) /
          static_cast<double>(audio_sample_rate));
    } else {
      throw std::runtime_error("LoadAudio: unsupported info type");
    }
  } else {
    core::audio::AudioInfo info;
    auto audio = from_memory_ ? core::audio::load(src, &info)
                              : core::audio::load(path, &info);

    if ((sampleRate_ > 0) && (info.sampleRate != sampleRate_)) {
      static std::
          unordered_map<LoadAudioResamplingQuality, core::audio::ResampleMode>
              sf_resampling_quality = {
                  {LoadAudioResamplingQuality::SincBest,
                   core::audio::ResampleMode::best},
                  {LoadAudioResamplingQuality::SincMedium,
                   core::audio::ResampleMode::medium},
                  {LoadAudioResamplingQuality::SincFastest,
                   core::audio::ResampleMode::fastest},
                  {LoadAudioResamplingQuality::ZeroOrderHold,
                   core::audio::ResampleMode::zeroOrderHold},
                  {LoadAudioResamplingQuality::Linear,
                   core::audio::ResampleMode::linear}};
      auto it = sf_resampling_quality.find(resamplingQuality_);
      if (it == sf_resampling_quality.end()) {
        throw std::runtime_error("LoadAudio: invalid resampling quality");
      }

      audio = core::audio::resample(
          audio, it->second, info.sampleRate, sampleRate_);
    }
    dst = audio;
  }
  return dst;
}
} // namespace op
} // namespace data
} // namespace mlx
