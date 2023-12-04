// Copyright © 2023 Apple Inc.

#include "mlx/data/core/image/ImagePrivate.h"

namespace mlx {
namespace data {
namespace core {
namespace image {

std::shared_ptr<Array> load(const std::string& path) {
  auto result = load_jpeg(path);
  if (result == nullptr) {
    result = load_stbi(path);
  }
  return result;
}

std::shared_ptr<Array> load(const std::shared_ptr<const Array>& contents) {
  auto result = load_jpeg(contents);
  if (result == nullptr) {
    result = load_stbi(contents);
  }
  return result;
}

ImageInfo info(const std::string& path) {
  return info_stbi(path);
}

ImageInfo info(const std::shared_ptr<const Array>& contents) {
  return info_stbi(contents);
}

bool save(const std::shared_ptr<const Array>& image, const std::string& path) {
  verify_image(image);
  return save_jpeg(image, path);
}

void verify_image(const std::shared_ptr<const Array>& image) {
  auto dimensions = image->shape().size();
  if (dimensions != 3) {
    throw std::runtime_error(
        "verifyImage: image must be 3 dimension Array (HWC)");
  }

  if (channels(image) == 0 || channels(image) > 4) {
    throw std::runtime_error("verifyImage: channels must be 0 <= c <= 4");
  }
}

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
