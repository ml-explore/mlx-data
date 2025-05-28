// Copyright © 2023-2025 Apple Inc.

#include <string>
#include <unordered_map>

namespace mlx {
namespace data {
namespace core {

std::string version();
std::unordered_map<std::string, std::string> libs_version();

} // namespace core
} // namespace data
} // namespace mlx
