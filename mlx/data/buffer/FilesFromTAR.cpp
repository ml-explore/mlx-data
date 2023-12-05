// Copyright © 2023 Apple Inc.

#include "mlx/data/buffer/FilesFromTAR.h"
#include "mlx/data/Array.h"
#include "mlx/data/Sample.h"
#include "mlx/data/core/TARReader.h"

namespace mlx {
namespace data {
namespace buffer {

FilesFromTAR::FilesFromTAR(
    const std::string& tarfile,
    bool nested,
    int num_threads) {
  core::TARReader tarreader(tarfile, nested, num_threads);
  files_ = tarreader.get_file_list();
}

Sample FilesFromTAR::get(int64_t idx) const {
  if (idx < 0 || idx >= files_.size()) {
    throw std::runtime_error("FilesFromTAR: index out of range");
  }
  Sample res;
  res["file"] = std::make_shared<Array>(files_[idx]);
  return res;
}

int64_t FilesFromTAR::size() const {
  return files_.size();
}

} // namespace buffer
} // namespace data
} // namespace mlx
