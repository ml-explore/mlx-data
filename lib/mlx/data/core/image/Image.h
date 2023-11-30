// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Array.h"

namespace mlx {
namespace data {
namespace core {
namespace image {

/// Image metadata
struct ImageInfo {
 public:
  int width;
  int height;
  int channels;
};

std::shared_ptr<Array> load(const std::string& path);
std::shared_ptr<Array> load(const std::shared_ptr<const Array>& contents);

ImageInfo info(const std::string& path);
ImageInfo info(const std::shared_ptr<const Array>& contents);

bool save(const std::shared_ptr<const Array>& image, const std::string& path);

/// Verify that the given Array is structured like an image:
/// three dimensions (w, h, c), last dimension looks like channels
/// and is 1, 3, or 4.
void verify_image(const std::shared_ptr<const Array>& image);

/// Return the width of the image.  Requires that `verifyImage()` be called
/// previously.
inline const int64_t width(const std::shared_ptr<const Array>& image) {
  return image->shape()[1];
}

/// Return the height of the image.  Requires that `verifyImage()` be called
/// previously.
inline const int64_t height(const std::shared_ptr<const Array>& image) {
  return image->shape()[0];
}

/// Return the channel count of the image.  Requires that `verifyImage()` be
/// called previously.
inline const int64_t channels(const std::shared_ptr<const Array>& image) {
  return image->shape()[2];
}

std::shared_ptr<Array> scale(
    const std::shared_ptr<const Array>& image,
    double scale);
std::shared_ptr<Array> resize(
    const std::shared_ptr<const Array>& image,
    int64_t dw,
    int64_t dh); // may alter aspect ratio
std::shared_ptr<Array> crop(
    const std::shared_ptr<const Array>& image,
    int64_t x,
    int64_t y,
    int64_t w,
    int64_t h);
std::shared_ptr<Array>
affine(const std::shared_ptr<const Array>& image, const float mx[6], bool crop);
std::shared_ptr<Array>
rotate(const std::shared_ptr<const Array>& image, double angle, bool crop);
std::shared_ptr<Array> hflip(const std::shared_ptr<const Array>& image);
std::shared_ptr<Array> channel_reduction(
    const std::shared_ptr<const Array>& image,
    const float bias,
    const float multiplier[3]);

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
