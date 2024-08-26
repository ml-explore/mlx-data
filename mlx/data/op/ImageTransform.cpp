// Copyright © 2023 Apple Inc.

#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>

#include "mlx/data/core/State.h"
#include "mlx/data/core/image/Image.h"
#include "mlx/data/core/video/Video.h"
#include "mlx/data/op/ImageTransform.h"

namespace mlx {
namespace data {
namespace op {

ImageTransformOp::ImageTransformOp(
    const std::string& ikey,
    const std::string& okey)
    : KeyTransformOp(ikey, okey) {}
std::shared_ptr<Array> ImageTransformOp::apply_key(
    const std::shared_ptr<const Array>& src) const {
  if (src->shape().size() == 4) {
    core::video::verify_video(src);
    return apply_video(src);
  } else {
    core::image::verify_image(src);
    return apply_image(src);
  }
}

std::shared_ptr<Array> ImageTransformOp::apply_video(
    const std::shared_ptr<const Array>& video) const {
  // default implementation of applyVideo just calls apply on each frame

  auto frame_count = core::video::frames(video);

  // transform the first frame to determine geometry for the result
  auto frame = apply_image(array::slice(video, 0));

  size_t expected_width = core::image::width(frame);
  size_t expected_height = core::image::height(frame);

  // allocate storage for the transformed video
  auto result = std::make_shared<Array>(
      ArrayType::UInt8,
      frame_count,
      static_cast<int64_t>(expected_height),
      static_cast<int64_t>(expected_width),
      core::image::channels(frame));

  // copy the frame back into the result video
  array::copy(array::slice(result, 0), frame);

  // process the rest of the frames
  for (int i = 1; i < frame_count; i++) {
    frame = apply_image(array::slice(video, i));

    if (core::image::width(frame) != expected_width ||
        core::image::height(frame) != expected_height) {
      throw std::runtime_error(
          std::string("applyVideo: frame size inconsistent during transform"));
    }

    array::copy(array::slice(result, i), frame);
  }

  return result;
}

ImageResizeSmallestSide::ImageResizeSmallestSide(
    const std::string& ikey,
    int64_t size,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), size_(size) {};

std::shared_ptr<Array> ImageResizeSmallestSide::apply_image(
    const std::shared_ptr<const Array>& image) const {
  if (size_ <= 0) {
    throw std::runtime_error(
        std::string("ImageResizeSmallestSide: illegal target size: ") +
        std::to_string(size_));
  }
  const int64_t w = core::image::width(image);
  const int64_t h = core::image::height(image);
  double scale;
  if (h > w) {
    scale = (double)size_ / w;
  } else {
    scale = (double)size_ / h;
  }
  return core::image::scale(image, scale);
}

ImageResize::ImageResize(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), w_(w), h_(h) {};

std::shared_ptr<Array> ImageResize::apply_image(
    const std::shared_ptr<const Array>& image) const {
  return core::image::resize(image, w_, h_);
}

ImageCenterCrop::ImageCenterCrop(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), w_(w), h_(h) {};

std::shared_ptr<Array> ImageCenterCrop::apply_image(
    const std::shared_ptr<const Array>& image) const {
  const int64_t w = core::image::width(image);
  const int64_t h = core::image::height(image);
  if (h_ > h || w_ > w) {
    throw std::runtime_error(
        "ImageCenterCrop: target image size larger than input image");
  }
  int64_t x = (w - w_) / 2;
  int64_t y = (h - h_) / 2;
  return core::image::crop(image, x, y, w_, h_);
}

ImageRandomCrop::ImageRandomCrop(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), w_(w), h_(h) {};

ImageRandomCrop::Parameters ImageRandomCrop::generate_random_crop_(
    int64_t w,
    int64_t h) const {
  if (h_ > h || w_ > w) {
    throw std::runtime_error(
        "ImageRandomCrop: target image size larger than input image");
  }
  std::uniform_int_distribution<int64_t> x_uniform{0, w - w_};
  std::uniform_int_distribution<int64_t> y_uniform{0, h - h_};
  auto state = core::get_state();
  return {
      x_uniform(state->randomGenerator),
      y_uniform(state->randomGenerator),
      w_,
      h_};
}

std::shared_ptr<Array> ImageRandomCrop::apply_image(
    const std::shared_ptr<const Array>& image) const {
  const int64_t w = core::image::width(image);
  const int64_t h = core::image::height(image);
  auto p = generate_random_crop_(w, h);
  return core::image::crop(image, p.tx, p.ty, p.tw, p.th);
}

std::shared_ptr<Array> ImageRandomCrop::apply_video(
    const std::shared_ptr<const Array>& video) const {
  // apply a consistent crop to every frame
  const int64_t w = core::video::width(video);
  const int64_t h = core::video::height(video);
  const int64_t frame_count = core::video::frames(video);
  const int64_t c = core::video::channels(video);
  auto p = generate_random_crop_(w, h);
  if (p.tw == 0 || p.th == 0) {
    return std::make_shared<Array>(video);
  }

  auto result =
      std::make_shared<Array>(ArrayType::UInt8, frame_count, p.th, p.tw, c);

  for (int i = 0; i < frame_count; i++) {
    auto frame =
        core::image::crop(array::slice(video, i), p.tx, p.ty, p.tw, p.th);
    array::copy(array::slice(result, i), frame);
  }

  return result;
}

ImageRandomAreaCrop::ImageRandomAreaCrop(
    const std::string& ikey,
    std::pair<float, float> area_range,
    std::pair<float, float> aspect_ratio_range,
    int num_trial,
    const std::string& okey)
    : ImageTransformOp(ikey, okey),
      areaRange_(area_range),
      aspectRatioRange_(aspect_ratio_range),
      numTrial_(num_trial) {
  if ((area_range.first > area_range.second) || (area_range.first <= 0) ||
      (area_range.second > 1.0)) {
    throw std::runtime_error("ImageRandomAreaCrop: invalid area range");
  }
  if ((aspect_ratio_range.first > aspect_ratio_range.second) ||
      (aspect_ratio_range.first <= 0)) {
    throw std::runtime_error("ImageRandomAreaCrop: invalid aspect ratio range");
  }
  if ((area_range.first * aspect_ratio_range.first > 1) ||
      (area_range.first > aspect_ratio_range.second)) {
    throw std::runtime_error(
        "ImageRandomAreaCrop: provided area range and aspect ratio range cannot be fullfilled");
  }
  if (numTrial_ <= 0) {
    throw std::runtime_error(
        "ImageRandomAreaCrop: number of trial must be positive");
  }
}

ImageRandomAreaCrop::Parameters ImageRandomAreaCrop::generate_random_crop_(
    int64_t w,
    int64_t h) const {
  const float wf = static_cast<float>(w);
  const float hf = static_cast<float>(h);
  float r = wf / hf; // aspect ratio

  if (w == 0 | h == 0) {
    return {0, 0, 0, 0};
  }
  auto state = core::get_state();

  int64_t wmin =
      std::ceil(std::sqrt(areaRange_.first * aspectRatioRange_.first) * wf);
  int64_t wmax = std::floor(std::min(
      std::sqrt(areaRange_.second * aspectRatioRange_.second) * wf, wf));
  std::uniform_int_distribution<int64_t> w_uniform{wmin, wmax};
  if (wmin > wmax) {
    // cannot fullfill constraints
    return {0, 0, 0, 0};
  }
  int64_t tw = 0;
  int64_t th = 0;
  for (int trial = 0; trial < numTrial_; trial++) {
    tw = w_uniform(state->randomGenerator);

    int64_t hmin = std::ceil(std::max(
        1.0f / (r * aspectRatioRange_.second) * tw,
        areaRange_.first * wf * hf / tw));
    int64_t hmax = std::floor(std::min(
        std::min(
            1.0f / (r * aspectRatioRange_.first) * tw,
            areaRange_.second * wf * hf / tw),
        hf));
    if (hmin > hmax) {
      continue;
    }
    std::uniform_int_distribution<int64_t> h_uniform{hmin, hmax};
    th = h_uniform(state->randomGenerator);

    float tr =
        static_cast<float>(tw) / static_cast<float>(th); // target aspect ratio

    if ((areaRange_.first * w * h > tw * th) ||
        (areaRange_.second * w * h < tw * th)) {
      continue;
    }
    if ((aspectRatioRange_.first * r > tr) ||
        (aspectRatioRange_.second * r < tr)) {
      continue;
    }
    if (tw <= 0 || tw > w) {
      continue;
    }
    if (th <= 0 || th > h) {
      continue;
    }
    break;
  }
  if ((tw == 0) || (th == 0)) {
    return {0, 0, 0, 0};
  }
  std::uniform_int_distribution<int64_t> x_uniform{0, w - tw};
  std::uniform_int_distribution<int64_t> y_uniform{0, h - th};
  int64_t tx = x_uniform(state->randomGenerator);
  int64_t ty = y_uniform(state->randomGenerator);
  return {tx, ty, tw, th};
}

std::shared_ptr<Array> ImageRandomAreaCrop::apply_image(
    const std::shared_ptr<const Array>& image) const {
  const int64_t w = core::image::width(image);
  const int64_t h = core::image::height(image);
  auto p = generate_random_crop_(w, h);
  if (p.tw == 0 || p.th == 0) {
    return std::make_shared<Array>(image);
  }
  return core::image::crop(image, p.tx, p.ty, p.tw, p.th);
}

std::shared_ptr<Array> ImageRandomAreaCrop::apply_video(
    const std::shared_ptr<const Array>& video) const {
  // apply a consistent crop to every frame
  const int64_t w = core::video::width(video);
  const int64_t h = core::video::height(video);
  const int64_t frame_count = core::video::frames(video);
  const int64_t c = core::video::channels(video);
  auto p = generate_random_crop_(w, h);
  if (p.tw == 0 || p.th == 0) {
    return std::make_shared<Array>(video);
  }

  auto result =
      std::make_shared<Array>(ArrayType::UInt8, frame_count, p.th, p.tw, c);

  for (int i = 0; i < frame_count; i++) {
    auto frame =
        core::image::crop(array::slice(video, i), p.tx, p.ty, p.tw, p.th);
    array::copy(array::slice(result, i), frame);
  }

  return result;
}

ImageRandomHFlip::ImageRandomHFlip(
    const std::string& ikey,
    float prob,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), prob_(prob) {};

std::shared_ptr<Array> ImageRandomHFlip::apply_image(
    const std::shared_ptr<const Array>& image) const {
  std::uniform_real_distribution<float> uniform{0, 1.0};
  auto state = core::get_state();
  if (uniform(state->randomGenerator) <= prob_) {
    return core::image::hflip(image);
  } else {
  }
  return std::make_shared<Array>(image);
}

std::shared_ptr<Array> ImageRandomHFlip::apply_video(
    const std::shared_ptr<const Array>& video) const {
  // apply a consistent flip to every frame
  std::uniform_real_distribution<float> uniform{0, 1.0};
  auto state = core::get_state();
  if (uniform(state->randomGenerator) <= prob_) {
    const int64_t w = core::video::width(video);
    const int64_t h = core::video::height(video);
    const int64_t frame_count = core::video::frames(video);
    const int64_t c = core::video::channels(video);

    auto result =
        std::make_shared<Array>(ArrayType::UInt8, frame_count, h, w, c);

    for (int i = 0; i < frame_count; i++) {
      auto frame = core::image::hflip(array::slice(video, i));
      array::copy(array::slice(result, i), frame);
    }

    return result;
  } else {
    return std::make_shared<Array>(video);
  }
}

ImageRotate::ImageRotate(
    const std::string& ikey,
    double angle,
    bool crop,
    const std::string& okey)
    : ImageTransformOp(ikey, okey), angle_(angle), crop_(crop) {};

std::shared_ptr<Array> ImageRotate::apply_image(
    const std::shared_ptr<const Array>& image) const {
  return core::image::rotate(image, angle_, crop_);
}

struct ImageChannelReductionSettings {
  float bias;
  float m[3];
};

/// Collection of Y' = R' * Kr + G' * Kg * B' * Kb + bias downmixes.  These are
/// typically defined for video and have a range constrained and biases output
/// (e.g. 16 - 235) but we are using full range (0 to 255) mixes here as is done
/// in e.g. OpenCV.
///
/// See also: https://en.wikipedia.org/wiki/YCbCr
static const std::unordered_map<std::string, ImageChannelReductionSettings>
    IMAGE_CHANNEL_REDUCTION_PRESETS = {
        /// the default mix, equivalent to cv.COLOR_RGB2GRAY and standard JFIF
        /// conversion, alias for rec601
        {"default", {0, {0.299, 0.587, 0.114}}},

        /// standard definition television conversion
        {"rec601", {0, {0.299, 0.587, 0.114}}},

        /// HDTV conversion, also used for sRGB -- thought to more closely match
        /// phosphers in modern displays
        {"rec709", {0, {0.2126, 0.7152, 0.0722}}},

        /// rec. 2020 coefficients
        {"rec2020", {0, {0.2627, 0.678, 0.0593}}},

        /// only the green channel
        {"green", {0, {0, 1, 0}}},
};

ImageChannelReduction::ImageChannelReduction(
    const std::string& ikey,
    std::string preset,
    const std::string& okey)
    : ImageTransformOp(ikey, okey) {
  auto settings = IMAGE_CHANNEL_REDUCTION_PRESETS.find(preset);

  if (settings == IMAGE_CHANNEL_REDUCTION_PRESETS.end()) {
    throw std::runtime_error(
        std::string("ImageChannelReduction: unable to find preset ") + preset);
  }

  this->bias_ = settings->second.bias;
  memcpy(this->m_, settings->second.m, sizeof(this->m_));
}

std::shared_ptr<Array> ImageChannelReduction::apply_image(
    const std::shared_ptr<const Array>& image) const {
  return core::image::channel_reduction(image, this->bias_, this->m_);
}

} // namespace op
} // namespace data
} // namespace mlx
