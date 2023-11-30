// Copyright © 2023 Apple Inc.

#include "mlx/data/core/image/ImagePrivate.h"

/* DEBUG: #define STBI_NEON 1 */
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace mlx {
namespace data {
namespace core {
namespace image {

std::shared_ptr<Array> load_stbi(const std::string& path) {
  ImageInfo info = info_stbi(path);
  int required_channels = info.channels;
  if (required_channels > 3) {
    required_channels = 3;
  }

  int w, h, c;
  unsigned char* img = nullptr;
  img = stbi_load(path.c_str(), &w, &h, &c, required_channels);

  if (!img) {
    throw std::runtime_error("load_stbi: could not load <" + path + ">");
  }

  std::vector<int64_t> shape = {h, w, required_channels};
  return std::make_shared<Array>(
      UInt8, shape, std::shared_ptr<void>((void*)img, stbi_image_free));
}

std::shared_ptr<Array> load_stbi(const std::shared_ptr<const Array> contents) {
  ImageInfo info = info_stbi(contents);
  int required_channels = info.channels;
  if (required_channels > 3) {
    required_channels = 3;
  }

  int w, h, c;
  unsigned char* img = nullptr;
  img = stbi_load_from_memory(
      contents->data<uint8_t>(),
      contents->size(),
      &w,
      &h,
      &c,
      required_channels);

  if (!img) {
    throw std::runtime_error("load_stbi: could not load from memory");
  }

  std::vector<int64_t> shape = {h, w, required_channels};
  return std::make_shared<Array>(
      UInt8, shape, std::shared_ptr<void>((void*)img, stbi_image_free));
}

ImageInfo info_stbi(const std::string& path) {
  int w, h, c;
  if (!stbi_info(path.c_str(), &w, &h, &c)) {
    return {};
  }
  return {
      .width = w,
      .height = h,
      .channels = c,
  };
}

ImageInfo info_stbi(const std::shared_ptr<const Array> contents) {
  int w, h, c;
  if (!stbi_info_from_memory(
          contents->data<uint8_t>(), contents->size(), &w, &h, &c)) {
    return {};
  }
  // clamp the number of channels to 3 -- no alpha
  if (c == 4) {
    c = 3;
  }
  return {
      .width = w,
      .height = h,
      .channels = c,
  };
}

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
