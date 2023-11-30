// Copyright © 2023 Apple Inc.

#pragma once

#include <string>
#include <unordered_map>

#include "mlx/data/Array.h"

namespace mlx {
namespace data {

typedef std::unordered_map<std::string, std::shared_ptr<Array>> Sample;

namespace sample {
std::vector<std::string> keys(const Sample& dict);
std::shared_ptr<Array>
check_key(const Sample& input, const std::string& key, ArrayType type);

} // namespace sample
} // namespace data
} // namespace mlx
