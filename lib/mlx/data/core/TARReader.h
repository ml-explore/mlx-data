// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/Array.h"

#include <fstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace mlx {
namespace data {
namespace core {

typedef std::unordered_map<std::string, std::pair<int64_t, size_t>>
    TARFileIndex;

class TARReader {
 public:
  TARReader(
      const std::string& filename,
      bool nested = false,
      int num_threads = 1);

  bool contains(const std::string& filename);
  std::shared_ptr<Array> get(const std::string& filename);
  std::vector<std::string> get_file_list();

 private:
  std::string filename_;
  TARFileIndex index_;
};

} // namespace core
} // namespace data
} // namespace mlx
