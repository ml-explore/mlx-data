// Copyright © 2023 Apple Inc.

#include "mlx/data/core/image/Image.h"

#include <cmath>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_UPSAMPLE STBIR_FILTER_TRIANGLE
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_TRIANGLE
#include <stb_image_resize2.h>

namespace mlx {
namespace data {
namespace core {
namespace image {

void verify_type(const std::shared_ptr<const Array>& image) {
  if (image->type() != UInt8) {
    throw std::invalid_argument("image must be of type UInt8");
  }
}

void verify_dimensions(int64_t w, int64_t h, int64_t c) {
  if (h <= 0 || w <= 0) {
    throw std::runtime_error(
        "image: cannot create image with 0 or negative dimension");
  }
  if (c <= 0 || c > 4) {
    throw std::runtime_error("image: channels must be 0 < c <= 4");
  }
}

std::shared_ptr<Array> scale(
    const std::shared_ptr<const Array>& image,
    double scale) {
  int64_t tw = std::lround(scale * width(image));
  int64_t th = std::lround(scale * height(image));
  return resize(image, tw, th);
}

std::shared_ptr<Array>
resize(const std::shared_ptr<const Array>& image, int64_t dw, int64_t dh) {
  int64_t w = width(image);
  int64_t h = height(image);
  int64_t c = channels(image);
  verify_dimensions(dw, dh, c);
  verify_type(image);
  auto result = std::make_shared<Array>(UInt8, dh, dw, c);
  if (!stbir_resize_uint8_linear(
          static_cast<unsigned char*>(image->data()),
          w,
          h,
          0,
          static_cast<unsigned char*>(result->data()),
          dw,
          dh,
          0,
          static_cast<stbir_pixel_layout>(c))) {
    throw std::runtime_error("image::resize: could not resize image");
  }
  return result;
}

std::shared_ptr<Array> crop(
    const std::shared_ptr<const Array>& image,
    int64_t x,
    int64_t y,
    int64_t w,
    int64_t h) {
  verify_dimensions(w, h, 3);
  auto result = array::sub(image, {y, x, 0}, {h, w, -1});
  return result;
}

std::shared_ptr<Array> affine(
    const std::shared_ptr<const Array>& image,
    const float mx[6],
    bool crop) {
  int64_t w = width(image);
  int64_t h = height(image);
  int64_t c = channels(image);
  int64_t tw = w;
  int64_t th = h;
  if (!crop) {
    tw = w * fabs(mx[0]) + h * fabs(mx[1]);
    th = h * fabs(mx[3]) + w * fabs(mx[4]);
  }
  const float twh = tw / 2.0;
  const float thh = th / 2.0;
  const float wh = w / 2.0;
  const float hh = h / 2.0;
  verify_dimensions(tw, th, c);
  verify_type(image);
  auto result = std::make_shared<Array>(UInt8, th, tw, c);
  auto src = (unsigned char*)image->data();
  auto dst = (unsigned char*)result->data();
  for (int64_t ty = 0; ty < th; ty++) {
    for (int64_t tx = 0; tx < tw; tx++) {
      int64_t x =
          (int64_t)(mx[0] * (tx - twh) + mx[1] * (ty - thh) + mx[2] + 0.5 + wh);
      int64_t y =
          (int64_t)(mx[3] * (tx - twh) + mx[4] * (ty - thh) + mx[5] + 0.5 + hh);
      if (x < 0 || y < 0 || x >= w || y >= h) {
        memset(dst + ty * tw * c + tx * c, 0, c);
      } else {
        memcpy(dst + ty * tw * c + tx * c, src + y * w * c + x * c, c);
      }
    }
  }
  return result;
}

std::shared_ptr<Array>
rotate(const std::shared_ptr<const Array>& image, double angle, bool crop) {
  const float pi = std::atan(1.0) * 4;
  float rangle = angle * pi / 180.;
  float c = std::cos(rangle);
  float s = std::sin(rangle);
  float mx[6] = {c, s, 0, -s, c, 0};
  return affine(image, mx, crop);
}

std::shared_ptr<Array> hflip(const std::shared_ptr<const Array>& image) {
  int64_t w = width(image);
  int64_t h = height(image);
  int64_t c = channels(image);
  verify_dimensions(w, h, c);
  verify_type(image);
  auto result = std::make_shared<Array>(UInt8, h, w, c);
  auto src = (unsigned char*)image->data();
  auto dst = (unsigned char*)result->data();
  for (int64_t y = 0; y < h; y++) {
    for (int64_t x = 0; x < w; x++) {
      for (int64_t k = 0; k < c; k++) {
        dst[y * w * c + x * c + k] = src[y * w * c + (w - x - 1) * c + k];
      }
    }
  }
  return result;
}

std::shared_ptr<Array> channel_reduction(
    const std::shared_ptr<const Array>& image,
    const float bias,
    const float multiplier[3]) {
  int64_t w = width(image);
  int64_t h = height(image);
  int64_t c = channels(image);
  if (c != 3) {
    throw std::runtime_error(
        "image::channelReduction: expected a 3 channel uint8 array");
  }
  verify_dimensions(w, h, 1);
  verify_type(image);
  auto result = std::make_shared<Array>(UInt8, h, w, 1);
  auto src = (unsigned char*)image->data();
  auto dst = (unsigned char*)result->data();

  const int scale = 256 * 256;
  int int_bias = bias * scale;
  int m[3];
  for (int i = 0; i < 3; i++) {
    m[i] = multiplier[i] * scale;
  }

  for (int64_t y = 0; y < h; y++) {
    for (int64_t x = 0; x < w; x++) {
      // v = r * m0 + g * m1 + b * m2 + bias
      int value = (src[y * w * 3 + x * 3 + 0] * m[0] +
                   src[y * w * 3 + x * 3 + 1] * m[1] +
                   src[y * w * 3 + x * 3 + 2] * m[2] + int_bias) /
          scale;

      // clamp to 8 bits
      value = value <= 255 ? value : 255;
      value = value >= 0 ? value : 0;

      dst[y * w + x] = value;
    }
  }
  return result;
}

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
