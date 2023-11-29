#include <stdexcept>

#include "mlx/data/Sample.h"

namespace mlx {
namespace data {
namespace sample {
std::vector<std::string> keys(const Sample& dict) {
  std::vector<std::string> keys;
  for (auto& kv : dict) {
    keys.push_back(kv.first);
  }
  return keys;
}

std::shared_ptr<Array>
check_key(const Sample& input, const std::string& key, ArrayType type) {
  auto it = input.find(key);
  if (it == input.end()) {
    throw std::runtime_error("key <" + key + "> expected");
  }
  auto value = it->second;
  if (type != ArrayType::Any && value->type() != type) {
    throw std::runtime_error("invalid Array type");
  }
  return value;
}
} // namespace sample
} // namespace data
} // namespace mlx
