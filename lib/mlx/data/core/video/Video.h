// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Array.h"

namespace mlx {
namespace data {
namespace core {
namespace video {

/// Video metadata
struct VideoInfo {
 public:
  int width;
  int height;
  int channels;
  int64_t frames;
};

std::shared_ptr<Array> load(const std::string& path);
std::shared_ptr<Array> load(const std::shared_ptr<const Array>& contents);

VideoInfo info(const std::string& path);
VideoInfo info(const std::shared_ptr<const Array>& contents);

/// Verify that the given Array is structured like a video:
/// four dimensions (f, w, h, c), last dimension looks like channels
/// and is 1, 3, or 4.
void verify_video(const std::shared_ptr<const Array>& video);

/// Return the width of the image.  Requires that `verifyVideo()` be called
/// previously.
inline const int64_t width(const std::shared_ptr<const Array>& video) {
  return video->shape()[2];
}

/// Return the height of the image.  Requires that `verifyVideo()` be called
/// previously.
inline const int64_t height(const std::shared_ptr<const Array>& video) {
  return video->shape()[1];
}

/// Return the channel count of the image.  Requires that `verifyVideo()` be
/// called previously.
inline const int64_t channels(const std::shared_ptr<const Array>& video) {
  return video->shape()[3];
}

/// Return the frame count of the image.  Requires that `verifyVideo()` be
/// called previously.
inline const int64_t frames(const std::shared_ptr<const Array>& video) {
  return video->shape()[0];
}

} // namespace video
} // namespace core
} // namespace data
} // namespace mlx
