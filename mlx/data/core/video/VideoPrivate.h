// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/core/video/Video.h"

#include <string>

namespace mlx {
namespace data {
namespace core {
namespace video {

// Opaque state
class VideoReaderState;

class VideoReader {
 public:
  VideoReader(const std::string& filename);
  VideoReader(const std::shared_ptr<const Array>& contents);

  ~VideoReader();

  VideoInfo info();
  std::shared_ptr<Array> read_frame(
      std::shared_ptr<Array> destination = nullptr);

 private:
  VideoReaderState* state_;
};

} // namespace video
} // namespace core
} // namespace data
} // namespace mlx
