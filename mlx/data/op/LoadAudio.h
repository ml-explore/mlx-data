// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/op/KeyTransform.h"

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

class LoadAudio : public KeyTransformOp {
 public:
  LoadAudio(
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      bool from_memory = false,
      LoadAudioInfo info_type = LoadAudioInfo::All,
      int sample_rate = 0,
      LoadAudioResamplingQuality resampling_quality =
          LoadAudioResamplingQuality::SincFastest,
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& x) const override;

 private:
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
