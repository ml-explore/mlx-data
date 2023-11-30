// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/core/image/Image.h"
#include "mlx/data/op/KeyTransform.h"

namespace mlx {
namespace data {
namespace op {

class ImageTransformOp : public KeyTransformOp {
 public:
  ImageTransformOp(const std::string& ikey, const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_key(
      const std::shared_ptr<const Array>& x) const override;

  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const = 0;

  /// apply the transform to every frame in a `Video`.  The default
  /// implementation simply calls `applyImage(Image)` for each frame.
  virtual std::shared_ptr<Array> apply_video(
      const std::shared_ptr<const Array>& image) const;
};

class ImageResizeSmallestSide : public ImageTransformOp {
 public:
  ImageResizeSmallestSide(
      const std::string& ikey,
      int64_t size,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;

 private:
  int64_t size_;
};

class ImageResize : public ImageTransformOp {
 public:
  ImageResize(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;

 private:
  int64_t w_;
  int64_t h_;
};

class ImageCenterCrop : public ImageTransformOp {
 public:
  ImageCenterCrop(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;

 private:
  int64_t w_;
  int64_t h_;
};

class ImageRandomCrop : public ImageTransformOp {
 public:
  ImageRandomCrop(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;
  virtual std::shared_ptr<Array> apply_video(
      const std::shared_ptr<const Array>& video) const override;

 private:
  int64_t w_;
  int64_t h_;

  struct Parameters {
    int64_t tx;
    int64_t ty;
    int64_t tw;
    int64_t th;
  };

  Parameters generate_random_crop_(int64_t w, int64_t h) const;
};

class ImageRandomAreaCrop : public ImageTransformOp {
 public:
  ImageRandomAreaCrop(
      const std::string& ikey,
      std::pair<float, float> area_range,
      std::pair<float, float> aspect_ratio_range,
      int num_trial = 10,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;
  virtual std::shared_ptr<Array> apply_video(
      const std::shared_ptr<const Array>& video) const override;

 private:
  std::pair<float, float> areaRange_;
  std::pair<float, float> aspectRatioRange_;
  int numTrial_;

  struct Parameters {
    int64_t tx;
    int64_t ty;
    int64_t tw;
    int64_t th;
  };

  Parameters generate_random_crop_(int64_t w, int64_t h) const;
};

class ImageRandomHFlip : public ImageTransformOp {
 public:
  ImageRandomHFlip(
      const std::string& ikey,
      float prob,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;
  virtual std::shared_ptr<Array> apply_video(
      const std::shared_ptr<const Array>& video) const override;

 private:
  float prob_;
};

class ImageRotate : public ImageTransformOp {
 public:
  ImageRotate(
      const std::string& ikey,
      double angle,
      bool crop = false,
      const std::string& okey = "");
  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;

 private:
  double angle_;
  bool crop_;
};

class ImageChannelReduction : public ImageTransformOp {
 public:
  ImageChannelReduction(
      const std::string& ikey,
      std::string preset = "default",
      const std::string& okey = "");

  virtual std::shared_ptr<Array> apply_image(
      const std::shared_ptr<const Array>& image) const override;

 private:
  float bias_;
  float m_[3];
};

} // namespace op
} // namespace data
} // namespace mlx
