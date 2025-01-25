// Copyright © 2024 Apple Inc.

#include "mlx/data/op/Replace.h"
#include "mlx/data/core/Utils.h"

namespace mlx {
namespace data {
namespace op {

Replace::Replace(
    const std::string& key,
    const std::string& old,
    const std::string& replacement,
    int count)
    : KeyTransformOp(key),
      old_(std::make_shared<Array>(old)),
      replacement_(std::make_shared<Array>(replacement)),
      count_(count) {}

std::shared_ptr<Array> Replace::apply_key(
    const std::shared_ptr<const Array>& src) const {
  return core::replace(src, old_, replacement_, count_);
}

ReplaceBytes::ReplaceBytes(
    const std::string& ikey,
    std::vector<std::string> byte_map,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), byte_map_(std::move(byte_map)) {
  while (byte_map_.size() < 256) {
    byte_map_.emplace_back("");
  }
}

std::shared_ptr<Array> ReplaceBytes::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::string result;
  // waste some space but ensure that we most often we do only 2 allocations
  result.reserve(2 * src->size() * src->itemsize());

  void* raw_data = src->data();
  uint8_t* byte_data = reinterpret_cast<uint8_t*>(raw_data);
  for (int64_t i = 0; i < src->size() * src->itemsize(); i++) {
    result += byte_map_[*byte_data];
    byte_data++;
  }

  return std::make_shared<Array>(result);
}

} // namespace op
} // namespace data
} // namespace mlx
