// Copyright © 2023-2025 Apple Inc.

#include <string>
#include <unordered_map>

namespace mlx {
namespace data {
namespace core {

std::string version();
std::unordered_map<std::string, bool> supported_libs();
std::unordered_map<std::string, std::string> supported_libs_version();

} // namespace core
} // namespace data
} // namespace mlx
