// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/core/audio/Audio.h"

namespace mlx {
namespace data {
namespace core {
namespace audio {

std::shared_ptr<Array> load_sndfile(const std::string& path, AudioInfo* info);
std::shared_ptr<Array> load_sndfile(
    const std::shared_ptr<const Array>& contents,
    AudioInfo* info);

AudioInfo info_sndfile(const std::string& path);
AudioInfo info_sndfile(const std::shared_ptr<const Array>& contents);

} // namespace audio
} // namespace core
} // namespace data
} // namespace mlx
