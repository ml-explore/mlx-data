// Copyright © 2023 Apple Inc.

#pragma once

#include <filesystem>
#include <functional>

#include "mlx/data/Array.h"
#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/core/Trie.h"
#include "mlx/data/op/LoadAudio.h"
#include "mlx/data/op/Op.h"
#include "mlx/data/op/Tokenize.h"

namespace mlx {
namespace data {

using LoadAudioInfo = op::LoadAudioInfo;
using LoadAudioResamplingQuality = op::LoadAudioResamplingQuality;
using TokenizeMode = op::TokenizeMode;

template <class T, class B>
class Dataset {
 public:
  Dataset(const std::shared_ptr<B>& self) : self_(self){};

  T filter_by_shape(
      const std::string& key,
      int dim,
      int64_t low = -1,
      int64_t high = -1) const;
  T filter_by_shape_if(
      bool cond,
      const std::string& key,
      int dim,
      int64_t low = -1,
      int64_t high = -1) const;

  T filter_key(const std::string& key, bool remove = false) const;
  T filter_key_if(bool cond, const std::string& key, bool remove = false) const;

  T image_center_crop(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;
  T image_center_crop_if(
      bool cond,
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;

  T image_channel_reduction(
      const std::string& ikey,
      std::string preset = "default",
      const std::string& okey = "") const;
  T image_channel_reduction_if(
      bool cond,
      const std::string& ikey,
      std::string preset = "default",
      const std::string& okey = "") const;

  T image_random_area_crop(
      const std::string& ikey,
      std::pair<float, float> area_range,
      std::pair<float, float> aspect_ratio_range,
      int num_trial = 10,
      const std::string& okey = "") const;
  T image_random_area_crop_if(
      bool cond,
      const std::string& ikey,
      std::pair<float, float> area_range,
      std::pair<float, float> aspect_ratio_range,
      int num_trial = 10,
      const std::string& okey = "") const;

  T image_random_crop(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;
  T image_random_crop_if(
      bool cond,
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;

  T image_random_h_flip(
      const std::string& ikey,
      float prob,
      const std::string& okey = "") const;
  T image_random_h_flip_if(
      bool cond,
      const std::string& ikey,
      float prob,
      const std::string& okey = "") const;

  T image_resize(
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;
  T image_resize_if(
      bool cond,
      const std::string& ikey,
      int64_t w,
      int64_t h,
      const std::string& okey = "") const;

  T image_resize_smallest_side(
      const std::string& ikey,
      int64_t size,
      const std::string& okey = "") const;
  T image_resize_smallest_side_if(
      bool cond,
      const std::string& ikey,
      int64_t size,
      const std::string& okey = "") const;

  T image_rotate(
      const std::string& ikey,
      double angle,
      bool crop = false,
      const std::string& okey = "") const;
  T image_rotate_if(
      bool cond,
      const std::string& ikey,
      double angle,
      bool crop = false,
      const std::string& okey = "") const;

  T key_transform(
      const std::string& ikey,
      std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
          op,
      const std::string& okey = "") const;
  T key_transform_if(
      bool cond,
      const std::string& ikey,
      std::function<std::shared_ptr<Array>(const std::shared_ptr<const Array>&)>
          op,
      const std::string& okey = "") const;

  T sample_transform(std::function<Sample(const Sample&)> op) const;
  T sample_transform_if(bool cond, std::function<Sample(const Sample&)> op)
      const;

  T load_audio(
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      bool from_memory = false,
      LoadAudioInfo info_type = LoadAudioInfo::All,
      int sample_rate = 0,
      LoadAudioResamplingQuality resampling_quality =
          LoadAudioResamplingQuality::SincFastest,
      const std::string& okey = "") const;
  T load_audio_if(
      bool cond,
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      bool from_memory = false,
      LoadAudioInfo info_type = LoadAudioInfo::All,
      int sample_rate = 0,
      LoadAudioResamplingQuality resampling_quality =
          LoadAudioResamplingQuality::SincFastest,
      const std::string& okey = "") const;

  T load_file(
      const std::string& ikey,
      const std::filesystem::path& prefix = "",
      const std::string& okey = "") const;
  T load_file_if(
      bool cond,
      const std::string& ikey,
      const std::filesystem::path& prefix = "",
      const std::string& okey = "") const;

  T load_image(
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      const std::string& format = "RGB",
      bool from_memory = false,
      const std::string& okey = "") const;
  T load_image_if(
      bool cond,
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      const std::string& format = "RGB",
      bool from_memory = false,
      const std::string& okey = "") const;

  T load_numpy(
      const std::string& ikey,
      const std::string& prefix = "",
      bool from_memory = false,
      const std::string& okey = "") const;
  T load_numpy_if(
      bool cond,
      const std::string& ikey,
      const std::string& prefix = "",
      bool from_memory = false,
      const std::string& okey = "") const;

  T load_video(
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      bool from_memory = false,
      const std::string& okey = "") const;
  T load_video_if(
      bool cond,
      const std::string& ikey,
      const std::string& prefix = "",
      bool info = false,
      bool from_memory = false,
      const std::string& okey = "") const;

  T pad(
      const std::string& ikey,
      int dim,
      int64_t lpad,
      int64_t rpad,
      double value,
      const std::string& okey = "") const;
  T pad_if(
      bool cond,
      const std::string& ikey,
      int dim,
      int64_t lpad,
      int64_t rpad,
      double value,
      const std::string& okey = "") const;

  T pad_to_multiple(
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "") const;
  T pad_to_multiple_if(
      bool cond,
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "") const;

  T pad_to_size(
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "") const;
  T pad_to_size_if(
      bool cond,
      const std::string& ikey,
      int dim,
      int64_t size,
      double value,
      const std::string& okey = "") const;
  T pad_to_size(
      const std::string& ikey,
      int dim,
      const std::vector<int64_t>& sizes,
      double value,
      const std::string& okey = "") const;
  T pad_to_size_if(
      bool cond,
      const std::string& ikey,
      int dim,
      const std::vector<int64_t>& sizes,
      double value,
      const std::string& okey = "") const;

  T read_from_tar(
      const std::string& tarkey,
      const std::string& ikey,
      const std::string& okey,
      const std::filesystem::path& prefix = "",
      const std::filesystem::path& tar_prefix = "",
      bool from_key = false,
      std::shared_ptr<core::FileFetcher> fetcher = nullptr,
      bool nested = false,
      int num_threads = 1) const;
  T read_from_tar_if(
      bool cond,
      const std::string& tarkey,
      const std::string& ikey,
      const std::string& okey,
      const std::filesystem::path& prefix = "",
      const std::filesystem::path& tar_prefix = "",
      bool from_key = false,
      std::shared_ptr<core::FileFetcher> fetcher = nullptr,
      bool nested = false,
      int num_threads = 1) const;

  T remove_value(
      const std::string& key,
      const std::string& size_key,
      int dim,
      double value,
      double pad) const;
  T remove_value_if(
      bool cond,
      const std::string& key,
      const std::string& size_key,
      int dim,
      double value,
      double pad) const;

  T rename_key(const std::string& ikey, const std::string& okey) const;
  T rename_key_if(bool cond, const std::string& ikey, const std::string& okey)
      const;

  T save_image(
      const std::string& image_key,
      const std::string& filename_key,
      const std::string& prefix = "",
      const std::string& filename_prefix = "") const;
  T save_image_if(
      bool cond,
      const std::string& image_key,
      const std::string& filename_key,
      const std::string& prefix = "",
      const std::string& filename_prefix = "") const;

  T shape(const std::string& ikey, const std::string& okey) const;
  T shape_if(bool cond, const std::string& ikey, const std::string& okey) const;
  T shape(const std::string& ikey, int dim, const std::string& okey) const;
  T shape_if(
      bool cond,
      const std::string& ikey,
      int dim,
      const std::string& okey) const;

  T shard(
      const std::string& ikey,
      int64_t n_shards,
      const std::string& okey = "") const;
  T shard_if(
      bool cond,
      const std::string& ikey,
      int64_t n_shards,
      const std::string& okey = "") const;

  T squeeze(const std::string& ikey, const std::string& okey = "") const;
  T squeeze_if(bool cond, const std::string& ikey, const std::string& okey = "")
      const;
  T squeeze(const std::string& ikey, int dim, const std::string& okey = "")
      const;
  T squeeze_if(
      bool cond,
      const std::string& ikey,
      int dim,
      const std::string& okey = "") const;
  T squeeze(
      const std::string& ikey,
      const std::vector<int>& dims,
      const std::string& okey = "") const;
  T squeeze_if(
      bool cond,
      const std::string& ikey,
      const std::vector<int>& dims,
      const std::string& okey = "") const;

  T tokenize(
      const std::string& ikey,
      std::shared_ptr<core::Trie<char>> trie,
      TokenizeMode mode,
      bool ignore_unk = false,
      const std::vector<double>& trie_key_scores = {},
      const std::string& okey = "") const;
  T tokenize_if(
      bool cond,
      const std::string& ikey,
      std::shared_ptr<core::Trie<char>> trie,
      TokenizeMode mode,
      bool ignore_unk = false,
      const std::vector<double>& trie_key_scores = {},
      const std::string& okey = "") const;

 protected:
  std::shared_ptr<B> self_;
  T transform_(std::shared_ptr<op::Op> op) const;
};

} // namespace data
} // namespace mlx
