// Copyright © 2023 Apple Inc.

#pragma once

#include <filesystem>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <utility>

#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/core/TARReader.h"
#include "mlx/data/op/Op.h"

namespace mlx {
namespace data {
namespace op {

class ReadFromTAR : public Op {
 public:
  ReadFromTAR(
      const std::string& tarkey,
      const std::string& ikey,
      const std::string& okey,
      const std::filesystem::path& prefix = "",
      const std::filesystem::path& tar_prefix = "",
      bool from_key = false,
      std::shared_ptr<core::FileFetcher> fetcher = nullptr,
      bool nested = false,
      int num_threads = 1);

  virtual Sample apply(const Sample& sample) const override;

 private:
  std::pair<
      std::shared_ptr<core::TARReader>,
      std::shared_ptr<core::FileFetcherHandle>>
  get_tar_reader_(const std::string& key) const;

  std::string tarkey_;
  std::string ikey_;
  std::string okey_;
  std::filesystem::path prefix_;
  std::filesystem::path tarPrefix_;
  bool fromKey_;
  std::shared_ptr<core::FileFetcher> fetcher_;
  bool nested_;
  int numThreads_;
  mutable std::unordered_map<std::string, std::shared_ptr<core::TARReader>>
      tars_;
  mutable std::shared_mutex mutex_;
};

} // namespace op
} // namespace data
} // namespace mlx
