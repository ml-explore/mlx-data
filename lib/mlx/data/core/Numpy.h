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

std::shared_ptr<Array> load_numpy(const std::string& filename);

/// @brief Read numpy (npy) array file.
///
/// See also:
/// https://numpy.org/doc/stable/reference/generated/numpy.lib.format.html
///
/// @param filename file to read
/// @return Array with the contents of the file
std::shared_ptr<Array> load_numpy(
    std::istream& stream,
    const std::string& filename = nullptr);

} // namespace core
} // namespace data
} // namespace mlx
