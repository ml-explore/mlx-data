// Copyright © 2023 Apple Inc.

#pragma once

#include <filesystem>
#include <istream>

#include "mlx/data/core/CSVReader.h"
#include "mlx/data/core/FileFetcher.h"
#include "mlx/data/stream/Compose.h"
#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

class CSVReader : public Stream {
 public:
  CSVReader(
      const std::string& filename,
      char sep = ',',
      char quote = '"',
      const std::filesystem::path& local_prefix = "",
      std::shared_ptr<core::FileFetcher> fetcher = nullptr);
  CSVReader(
      const std::shared_ptr<std::istream>& f,
      char sep = ',',
      char quote = '"',
      std::shared_ptr<core::FileFetcherHandle> file_handle = nullptr);
  virtual Sample next() const override;
  void reset() override;

 private:
  std::unique_ptr<core::CSVReader> csv_;
  std::vector<std::string> keys_;
  std::shared_ptr<core::FileFetcherHandle> fileHandle_;
  mutable std::mutex mutex_;
};

class CSVReaderFromKey : public Compose {
 public:
  CSVReaderFromKey(
      std::shared_ptr<Stream> stream,
      const std::string& key,
      char sep = ',',
      char quote = '"',
      bool from_memory = false,
      const std::filesystem::path& local_prefix = "",
      const std::shared_ptr<core::FileFetcher>& fetcher = nullptr);
};

} // namespace stream
} // namespace data
} // namespace mlx
