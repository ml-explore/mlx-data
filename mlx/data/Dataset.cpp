// Copyright © 2023 Apple Inc.

#include "mlx/data/Dataset.h"
#include "mlx/data/Buffer.h"
#include "mlx/data/Stream.h"
#include "mlx/data/buffer/Buffer.h"
#include "mlx/data/buffer/Transform.h"
#include "mlx/data/stream/Stream.h"
#include "mlx/data/stream/Transform.h"

#include "mlx/data/op/FilterByShape.h"
#include "mlx/data/op/FilterKey.h"
#include "mlx/data/op/ImageTransform.h"
#include "mlx/data/op/KeyTransform.h"
#include "mlx/data/op/LoadAudio.h"
#include "mlx/data/op/LoadFile.h"
#include "mlx/data/op/LoadImage.h"
#include "mlx/data/op/LoadNumpy.h"
#include "mlx/data/op/LoadVideo.h"
#include "mlx/data/op/Pad.h"
#include "mlx/data/op/ReadFromTAR.h"
#include "mlx/data/op/RemoveValue.h"
#include "mlx/data/op/RenameKey.h"
#include "mlx/data/op/Replace.h"
#include "mlx/data/op/SampleTransform.h"
#include "mlx/data/op/SaveImage.h"
#include "mlx/data/op/Shape.h"
#include "mlx/data/op/Shard.h"
#include "mlx/data/op/Slice.h"
#include "mlx/data/op/Squeeze.h"
#include "mlx/data/op/Tokenize.h"

namespace mlx {
namespace data {

template <class T, class B>
T Dataset<T, B>::filter_by_shape(
    const std::string& key,
    int dim,
    int64_t low,
    int64_t high) const {
  return transform_(std::make_shared<op::FilterByShape>(key, dim, low, high));
}

template <class T, class B>
T Dataset<T, B>::filter_by_shape_if(
    bool cond,
    const std::string& key,
    int dim,
    int64_t low,
    int64_t high) const {
  if (cond) {
    return transform_(std::make_shared<op::FilterByShape>(key, dim, low, high));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::filter_key(const std::string& key, bool remove) const {
  return transform_(std::make_shared<op::FilterKey>(key, remove));
}

template <class T, class B>
T Dataset<T, B>::filter_key_if(bool cond, const std::string& key, bool remove)
    const {
  if (cond) {
    return transform_(std::make_shared<op::FilterKey>(key, remove));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_center_crop(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageCenterCrop>(ikey, w, h, okey));
}

template <class T, class B>
T Dataset<T, B>::image_center_crop_if(
    bool cond,
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::ImageCenterCrop>(ikey, w, h, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_channel_reduction(
    const std::string& ikey,
    std::string preset,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::ImageChannelReduction>(ikey, preset, okey));
}

template <class T, class B>
T Dataset<T, B>::image_channel_reduction_if(
    bool cond,
    const std::string& ikey,
    std::string preset,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::ImageChannelReduction>(ikey, preset, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_random_area_crop(
    const std::string& ikey,
    std::pair<float, float> areaRange,
    std::pair<float, float> aspectRatioRange,
    int numTrial,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageRandomAreaCrop>(
      ikey, areaRange, aspectRatioRange, numTrial, okey));
}

template <class T, class B>
T Dataset<T, B>::image_random_area_crop_if(
    bool cond,
    const std::string& ikey,
    std::pair<float, float> areaRange,
    std::pair<float, float> aspectRatioRange,
    int numTrial,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::ImageRandomAreaCrop>(
        ikey, areaRange, aspectRatioRange, numTrial, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_random_crop(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageRandomCrop>(ikey, w, h, okey));
}

template <class T, class B>
T Dataset<T, B>::image_random_crop_if(
    bool cond,
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::ImageRandomCrop>(ikey, w, h, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_random_h_flip(
    const std::string& ikey,
    float prob,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageRandomHFlip>(ikey, prob, okey));
}

template <class T, class B>
T Dataset<T, B>::image_random_h_flip_if(
    bool cond,
    const std::string& ikey,
    float prob,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::ImageRandomHFlip>(ikey, prob, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_resize(
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageResize>(ikey, w, h, okey));
}

template <class T, class B>
T Dataset<T, B>::image_resize_if(
    bool cond,
    const std::string& ikey,
    int64_t w,
    int64_t h,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::ImageResize>(ikey, w, h, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_resize_smallest_side(
    const std::string& ikey,
    int64_t size,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::ImageResizeSmallestSide>(ikey, size, okey));
}

template <class T, class B>
T Dataset<T, B>::image_resize_smallest_side_if(
    bool cond,
    const std::string& ikey,
    int64_t size,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::ImageResizeSmallestSide>(ikey, size, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::image_rotate(
    const std::string& ikey,
    double angle,
    bool crop,
    const std::string& okey) const {
  return transform_(std::make_shared<op::ImageRotate>(ikey, angle, crop, okey));
}

template <class T, class B>
T Dataset<T, B>::image_rotate_if(
    bool cond,
    const std::string& ikey,
    double angle,
    bool crop,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::ImageRotate>(ikey, angle, crop, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::key_transform(
    const std::string& ikey,
    std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
        op,
    const std::string& okey) const {
  return transform_(std::make_shared<op::KeyTransform>(ikey, op, okey));
}

template <class T, class B>
T Dataset<T, B>::key_transform_if(
    bool cond,
    const std::string& ikey,
    std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
        op,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::KeyTransform>(ikey, op, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::sample_transform(
    std::function<Sample(const Sample&)> op) const {
  return transform_(std::make_shared<op::SampleTransform>(op));
}

template <class T, class B>
T Dataset<T, B>::sample_transform_if(
    bool cond,
    std::function<Sample(const Sample&)> op) const {
  if (cond) {
    return transform_(std::make_shared<op::SampleTransform>(op));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::load_audio(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool fromMemory,
    LoadAudioInfo infoType,
    int sampleRate,
    LoadAudioResamplingQuality resamplingQuality,
    const std::string& okey) const {
  return transform_(std::make_shared<op::LoadAudio>(
      ikey,
      prefix,
      info,
      fromMemory,
      infoType,
      sampleRate,
      resamplingQuality,
      okey));
}

template <class T, class B>
T Dataset<T, B>::load_audio_if(
    bool cond,
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool fromMemory,
    LoadAudioInfo infoType,
    int sampleRate,
    LoadAudioResamplingQuality resamplingQuality,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::LoadAudio>(
        ikey,
        prefix,
        info,
        fromMemory,
        infoType,
        sampleRate,
        resamplingQuality,
        okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::load_file(
    const std::string& ikey,
    const std::filesystem::path& prefix,
    const std::string& okey) const {
  return transform_(std::make_shared<op::LoadFile>(ikey, prefix, okey));
}

template <class T, class B>
T Dataset<T, B>::load_file_if(
    bool cond,
    const std::string& ikey,
    const std::filesystem::path& prefix,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::LoadFile>(ikey, prefix, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::load_image(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    const std::string& format,
    bool fromMemory,
    const std::string& okey) const {
  return transform_(std::make_shared<op::LoadImage>(
      ikey, prefix, info, format, fromMemory, okey));
}

template <class T, class B>
T Dataset<T, B>::load_image_if(
    bool cond,
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    const std::string& format,
    bool fromMemory,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::LoadImage>(
        ikey, prefix, info, format, fromMemory, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::load_numpy(
    const std::string& ikey,
    const std::string& prefix,
    bool fromMemory,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::LoadNumpy>(ikey, prefix, fromMemory, okey));
}

template <class T, class B>
T Dataset<T, B>::load_numpy_if(
    bool cond,
    const std::string& ikey,
    const std::string& prefix,
    bool fromMemory,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::LoadNumpy>(ikey, prefix, fromMemory, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::load_video(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool fromMemory,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::LoadVideo>(ikey, prefix, info, fromMemory, okey));
}

template <class T, class B>
T Dataset<T, B>::load_video_if(
    bool cond,
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool fromMemory,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::LoadVideo>(ikey, prefix, info, fromMemory, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::pad(
    const std::string& ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::Pad>(ikey, dim, lpad, rpad, value, okey));
}

template <class T, class B>
T Dataset<T, B>::pad_if(
    bool cond,
    const std::string& ikey,
    int dim,
    int64_t lpad,
    int64_t rpad,
    double value,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::Pad>(ikey, dim, lpad, rpad, value, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::pad_to_multiple(
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::PadToMultiple>(ikey, dim, size, value, okey));
}

template <class T, class B>
T Dataset<T, B>::pad_to_multiple_if(
    bool cond,
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::PadToMultiple>(ikey, dim, size, value, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::pad_to_size(
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::PadToSize>(ikey, dim, size, value, okey));
}

template <class T, class B>
T Dataset<T, B>::pad_to_size_if(
    bool cond,
    const std::string& ikey,
    int dim,
    int64_t size,
    double value,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::PadToSize>(ikey, dim, size, value, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::pad_to_size(
    const std::string& ikey,
    int dim,
    const std::vector<int64_t>& sizes,
    double value,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::PadToSize>(ikey, dim, sizes, value, okey));
}

template <class T, class B>
T Dataset<T, B>::pad_to_size_if(
    bool cond,
    const std::string& ikey,
    int dim,
    const std::vector<int64_t>& sizes,
    double value,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::PadToSize>(ikey, dim, sizes, value, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::random_slice(
    const std::string& ikey,
    int dim,
    int64_t size,
    const std::string& okey) const {
  return transform_(std::make_shared<op::RandomSlice>(ikey, dim, size, okey));
}

template <class T, class B>
T Dataset<T, B>::random_slice_if(
    bool cond,
    const std::string& ikey,
    int dim,
    int64_t size,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::RandomSlice>(ikey, dim, size, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::random_slice(
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> sizes,
    const std::string& okey) const {
  return transform_(std::make_shared<op::RandomSlice>(ikey, dims, sizes, okey));
}

template <class T, class B>
T Dataset<T, B>::random_slice_if(
    bool cond,
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> sizes,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::RandomSlice>(ikey, dims, sizes, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::read_from_tar(
    const std::string& tarkey,
    const std::string& ikey,
    const std::string& okey,
    const std::filesystem::path& prefix,
    const std::filesystem::path& tarPrefix,
    bool fromKey,
    std::shared_ptr<core::FileFetcher> fetcher,
    bool nested,
    int numThreads) const {
  return transform_(std::make_shared<op::ReadFromTAR>(
      tarkey,
      ikey,
      okey,
      prefix,
      tarPrefix,
      fromKey,
      fetcher,
      nested,
      numThreads));
}

template <class T, class B>
T Dataset<T, B>::read_from_tar_if(
    bool cond,
    const std::string& tarkey,
    const std::string& ikey,
    const std::string& okey,
    const std::filesystem::path& prefix,
    const std::filesystem::path& tarPrefix,
    bool fromKey,
    std::shared_ptr<core::FileFetcher> fetcher,
    bool nested,
    int numThreads) const {
  if (cond) {
    return transform_(std::make_shared<op::ReadFromTAR>(
        tarkey,
        ikey,
        okey,
        prefix,
        tarPrefix,
        fromKey,
        fetcher,
        nested,
        numThreads));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::remove_value(
    const std::string& key,
    const std::string& size_key,
    int dim,
    double value,
    double pad) const {
  return transform_(
      std::make_shared<op::RemoveValue>(key, size_key, dim, value, pad));
}

template <class T, class B>
T Dataset<T, B>::remove_value_if(
    bool cond,
    const std::string& key,
    const std::string& size_key,
    int dim,
    double value,
    double pad) const {
  if (cond) {
    return transform_(
        std::make_shared<op::RemoveValue>(key, size_key, dim, value, pad));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::replace(
    const std::string& key,
    const std::string& old,
    const std::string& replacement,
    int count) {
  return transform_(
      std::make_shared<op::Replace>(key, old, replacement, count));
}

template <class T, class B>
T Dataset<T, B>::replace_if(
    bool cond,
    const std::string& key,
    const std::string& old,
    const std::string& replacement,
    int count) {
  if (cond) {
    return transform_(
        std::make_shared<op::Replace>(key, old, replacement, count));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::replace_bytes(
    const std::string& ikey,
    std::vector<std::string> byte_map,
    const std::string& okey) {
  return transform_(
      std::make_shared<op::ReplaceBytes>(ikey, std::move(byte_map), okey));
}

template <class T, class B>
T Dataset<T, B>::replace_bytes_if(
    bool cond,
    const std::string& ikey,
    std::vector<std::string> byte_map,
    const std::string& okey) {
  if (cond) {
    return transform_(
        std::make_shared<op::ReplaceBytes>(ikey, std::move(byte_map), okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::rename_key(const std::string& ikey, const std::string& okey)
    const {
  return transform_(std::make_shared<op::RenameKey>(ikey, okey));
}

template <class T, class B>
T Dataset<T, B>::rename_key_if(
    bool cond,
    const std::string& ikey,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::RenameKey>(ikey, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::save_image(
    const std::string& imageKey,
    const std::string& filenameKey,
    const std::string& prefix,
    const std::string& filenamePrefix) const {
  return transform_(std::make_shared<op::SaveImage>(
      imageKey, filenameKey, prefix, filenamePrefix));
}

template <class T, class B>
T Dataset<T, B>::save_image_if(
    bool cond,
    const std::string& imageKey,
    const std::string& filenameKey,
    const std::string& prefix,
    const std::string& filenamePrefix) const {
  if (cond) {
    return transform_(std::make_shared<op::SaveImage>(
        imageKey, filenameKey, prefix, filenamePrefix));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::shape(const std::string& ikey, const std::string& okey) const {
  return transform_(std::make_shared<op::Shape>(ikey, okey));
}

template <class T, class B>
T Dataset<T, B>::shape_if(
    bool cond,
    const std::string& ikey,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Shape>(ikey, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::shape(
    const std::string& ikey,
    int dim,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Shape>(ikey, dim, okey));
}

template <class T, class B>
T Dataset<T, B>::shape_if(
    bool cond,
    const std::string& ikey,
    int dim,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Shape>(ikey, dim, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::shard(
    const std::string& ikey,
    int64_t nShards,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Shard>(ikey, nShards, okey));
}

template <class T, class B>
T Dataset<T, B>::shard_if(
    bool cond,
    const std::string& ikey,
    int64_t nShards,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Shard>(ikey, nShards, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::squeeze(const std::string& ikey, const std::string& okey)
    const {
  return transform_(std::make_shared<op::Squeeze>(ikey, okey));
}

template <class T, class B>
T Dataset<T, B>::squeeze_if(
    bool cond,
    const std::string& ikey,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Squeeze>(ikey, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::squeeze(
    const std::string& ikey,
    int dim,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Squeeze>(ikey, dim, okey));
}

template <class T, class B>
T Dataset<T, B>::squeeze_if(
    bool cond,
    const std::string& ikey,
    int dim,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Squeeze>(ikey, dim, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::squeeze(
    const std::string& ikey,
    const std::vector<int>& dims,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Squeeze>(ikey, dims, okey));
}

template <class T, class B>
T Dataset<T, B>::squeeze_if(
    bool cond,
    const std::string& ikey,
    const std::vector<int>& dims,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Squeeze>(ikey, dims, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::slice(
    const std::string& ikey,
    int dim,
    int64_t start,
    int64_t end,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Slice>(ikey, dim, start, end, okey));
}

template <class T, class B>
T Dataset<T, B>::slice_if(
    bool cond,
    const std::string& ikey,
    int dim,
    int64_t start,
    int64_t end,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Slice>(ikey, dim, start, end, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::slice(
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> starts,
    std::vector<int64_t> ends,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::Slice>(ikey, dims, starts, ends, okey));
}

template <class T, class B>
T Dataset<T, B>::slice_if(
    bool cond,
    const std::string& ikey,
    std::vector<int> dims,
    std::vector<int64_t> starts,
    std::vector<int64_t> ends,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::Slice>(ikey, dims, starts, ends, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::tokenize(
    const std::string& ikey,
    std::shared_ptr<core::Trie<char>> trie,
    TokenizeMode mode,
    bool ignoreUnk,
    const std::vector<double>& trieKeyScores,
    const std::string& okey) const {
  return transform_(std::make_shared<op::Tokenize>(
      ikey, trie, mode, ignoreUnk, trieKeyScores, okey));
}

template <class T, class B>
T Dataset<T, B>::tokenize_if(
    bool cond,
    const std::string& ikey,
    std::shared_ptr<core::Trie<char>> trie,
    TokenizeMode mode,
    bool ignoreUnk,
    const std::vector<double>& trieKeyScores,
    const std::string& okey) const {
  if (cond) {
    return transform_(std::make_shared<op::Tokenize>(
        ikey, trie, mode, ignoreUnk, trieKeyScores, okey));
  } else {
    return T(self_);
  }
}

template <class T, class B>
T Dataset<T, B>::tokenize_bpe(
    const std::string& ikey,
    std::shared_ptr<const core::Trie<char>> symbols,
    std::shared_ptr<const core::BPEMerges> merges,
    const std::string& okey) const {
  return transform_(
      std::make_shared<op::BPETokenize>(ikey, symbols, merges, okey));
}

template <class T, class B>
T Dataset<T, B>::tokenize_bpe_if(
    bool cond,
    const std::string& ikey,
    std::shared_ptr<const core::Trie<char>> symbols,
    std::shared_ptr<const core::BPEMerges> merges,
    const std::string& okey) const {
  if (cond) {
    return transform_(
        std::make_shared<op::BPETokenize>(ikey, symbols, merges, okey));
  } else {
    return T(self_);
  }
}

// Implement Stream
template <>
Stream Dataset<Stream, stream::Stream>::transform_(
    std::shared_ptr<op::Op> op) const {
  return Stream(std::make_shared<stream::Transform>(self_, op));
}
template class Dataset<Stream, stream::Stream>;

// Implement Buffer
template <>
Buffer Dataset<Buffer, buffer::Buffer>::transform_(
    std::shared_ptr<op::Op> op) const {
  return Buffer(std::make_shared<buffer::Transform>(self_, op));
}
template class Dataset<Buffer, buffer::Buffer>;

} // namespace data
} // namespace mlx
