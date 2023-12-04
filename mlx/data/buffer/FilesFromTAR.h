// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/buffer/Buffer.h"

namespace mlx {
namespace data {
namespace buffer {

class FilesFromTAR : public Buffer {
 public:
  FilesFromTAR(
      const std::string& tarfile,
      bool nested = false,
      int num_threads = 1);

  Sample get(int64_t idx) const override;
  virtual int64_t size() const override;

 private:
  std::vector<std::string> files_;
};

} // namespace buffer
} // namespace data
} // namespace mlx
