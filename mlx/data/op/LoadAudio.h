// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

enum class LoadAudioInfo {
  All,
  NumFrames,
  NumChannels,
  SampleRate,
  NumSeconds
};

enum class LoadAudioResamplingQuality {
  SincBest,
  SincMedium,
  SincFastest,
  ZeroOrderHold,
  Linear,
};

class LoadAudio : public Op {
 public:
  LoadAudio(
      const std::string& ikey,
      const std::string& prefix = "",
      // if info is true, provides info
      // either in infokey (if provided)
      // or in okey (in which case no audio will be loaded)
      bool info = false,
      bool from_memory = false,
      LoadAudioInfo info_type = LoadAudioInfo::All,
      int sample_rate = 0,
      LoadAudioResamplingQuality resampling_quality =
          LoadAudioResamplingQuality::SincFastest,
      const std::string& infokey = "",
      const std::string& okey = "");

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::string iKey_;
  std::string oKey_;
  std::string infoKey_;
  std::string prefix_;
  bool info_;
  bool from_memory_;
  LoadAudioInfo infoType_;
  int sampleRate_;
  LoadAudioResamplingQuality resamplingQuality_;
};

} // namespace op
} // namespace data
} // namespace mlx
