#include "mlx/data/op/Squeeze.h"

namespace mlx {
namespace data {
namespace op {
Squeeze::Squeeze(const std::string& ikey, const std::string& okey)
    : KeyTransformOp(ikey, okey) {}
Squeeze::Squeeze(const std::string& ikey, int dim, const std::string& okey)
    : KeyTransformOp(ikey, okey), dims_({dim}) {}
Squeeze::Squeeze(
    const std::string& ikey,
    const std::vector<int>& dims,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), dims_(dims) {}

std::shared_ptr<Array> Squeeze::apply_key(
    const std::shared_ptr<const Array>& src) const {
  return mlx::data::array::squeeze(src, dims_);
}
} // namespace op
} // namespace data
} // namespace mlx
