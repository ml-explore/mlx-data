#include "mlx/data/op/RemoveValue.h"
#include "mlx/data/core/Utils.h"

namespace mlx {
namespace data {
namespace op {
RemoveValue::RemoveValue(
    const std::string& key,
    const std::string& size_key,
    int dim,
    double value,
    double pad)
    : key_(key), size_key_(size_key), dim_(dim), value_(value), pad_(pad) {}

Sample RemoveValue::apply(const Sample& sample) const {
  auto array = sample::check_key(sample, key_, ArrayType::Any);
  auto size_array = sample::check_key(sample, size_key_, ArrayType::Int64);
  std::tie(array, size_array) =
      core::remove(array, size_array, dim_, value_, pad_);
  auto new_sample = sample;
  new_sample[key_] = array;
  new_sample[size_key_] = size_array;
  return new_sample;
}
} // namespace op
} // namespace data
} // namespace mlx
