// Copyright © 2023 Apple Inc.

#include <cmath>
#include <filesystem>
#include <unordered_map>

#include "mlx/data/core/audio/Audio.h"
#include "mlx/data/op/LoadAudio.h"

namespace {
std::shared_ptr<mlx::data::Array> extract_audio_info(
    const mlx::data::core::audio::AudioInfo& audio_info,
    mlx::data::op::LoadAudioInfo info_type) {
  std::shared_ptr<mlx::data::Array> info;
  auto audio_length = audio_info.frames;
  auto audio_channels = audio_info.channels;
  auto audio_sample_rate = audio_info.sampleRate;
  if (info_type == mlx::data::op::LoadAudioInfo::All) {
    std::vector<int64_t> all_info(
        {audio_length, audio_channels, audio_sample_rate});
    info = std::make_shared<mlx::data::Array>(all_info);
  } else if (info_type == mlx::data::op::LoadAudioInfo::NumFrames) {
    info = std::make_shared<mlx::data::Array>(audio_length);
  } else if (info_type == mlx::data::op::LoadAudioInfo::NumChannels) {
    info = std::make_shared<mlx::data::Array>(audio_channels);
  } else if (info_type == mlx::data::op::LoadAudioInfo::SampleRate) {
    info = std::make_shared<mlx::data::Array>(audio_sample_rate);
  } else if (info_type == mlx::data::op::LoadAudioInfo::NumSeconds) {
    info = std::make_shared<mlx::data::Array>(
        static_cast<double>(audio_length) /
        static_cast<double>(audio_sample_rate));
  } else {
    throw std::runtime_error("LoadAudio: unsupported info type");
  }
  return info;
}
mlx::data::core::audio::ResampleMode convert_resample_mode(
    mlx::data::op::LoadAudioResamplingQuality resample_mode) {
  static std::unordered_map<
      mlx::data::op::LoadAudioResamplingQuality,
      mlx::data::core::audio::ResampleMode>
      sf_resampling_quality = {
          {mlx::data::op::LoadAudioResamplingQuality::SincBest,
           mlx::data::core::audio::ResampleMode::best},
          {mlx::data::op::LoadAudioResamplingQuality::SincMedium,
           mlx::data::core::audio::ResampleMode::medium},
          {mlx::data::op::LoadAudioResamplingQuality::SincFastest,
           mlx::data::core::audio::ResampleMode::fastest},
          {mlx::data::op::LoadAudioResamplingQuality::ZeroOrderHold,
           mlx::data::core::audio::ResampleMode::zeroOrderHold},
          {mlx::data::op::LoadAudioResamplingQuality::Linear,
           mlx::data::core::audio::ResampleMode::linear}};
  auto it = sf_resampling_quality.find(resample_mode);
  if (it == sf_resampling_quality.end()) {
    throw std::runtime_error("LoadAudio: invalid resampling quality");
  }
  return it->second;
}
} // namespace

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
    const std::string& infokey,
    const std::string& okey)
    : Op(),
      iKey_(ikey),
      oKey_(okey),
      infoKey_(infokey),
      prefix_(prefix),
      info_(info),
      from_memory_(from_memory),
      infoType_(info_type),
      sampleRate_(sample_rate),
      resamplingQuality_(resampling_quality) {}

Sample LoadAudio::apply(const Sample& sample) const {
  auto src = sample::check_key(sample, iKey_, ArrayType::Any);
  auto okey = (oKey_.empty() ? iKey_ : oKey_);
  auto res = sample;

  std::filesystem::path path;
  if (!from_memory_) {
    if (src->type() != ArrayType::Int8) {
      throw std::runtime_error("LoadAudio: expected filename (int8 array)");
    }
    path = prefix_;
    std::string filename(reinterpret_cast<char*>(src->data()), src->size());
    path /= filename;
  }

  std::shared_ptr<Array> info;
  std::shared_ptr<Array> dst;

  // need to return only info?
  if (info_ && infoKey_.empty()) {
    auto audio_info =
        from_memory_ ? core::audio::info(src) : core::audio::info(path);
    res[okey] = extract_audio_info(audio_info, infoType_);
  } else {
    core::audio::AudioInfo audio_info;
    auto audio = from_memory_ ? core::audio::load(src, &audio_info)
                              : core::audio::load(path, &audio_info);
    audio = core::audio::resample(
        audio,
        convert_resample_mode(resamplingQuality_),
        audio_info.sampleRate,
        sampleRate_);

    if (!infoKey_.empty()) {
      res[infoKey_] = extract_audio_info(audio_info, infoType_);
    }
    res[okey] = audio;
  }
  return res;
}
} // namespace op
} // namespace data
} // namespace mlx
