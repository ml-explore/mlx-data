#include "mlx/data/op/Shard.h"

namespace mlx {
namespace data {
namespace op {
Shard::Shard(const std::string& ikey, int64_t n_shards, const std::string& okey)
    : KeyTransformOp(ikey, okey), nShards_(n_shards) {}

std::shared_ptr<Array> Shard::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::vector<int64_t> shape = src->shape();
  if (shape.size() > 0) {
    shape[0] = -1;
    shape.insert(shape.begin(), nShards_);
    return mlx::data::array::reshape(src, shape);
  } else {
    return mlx::data::array::clone(src);
  }
}
} // namespace op
} // namespace data
} // namespace mlx
