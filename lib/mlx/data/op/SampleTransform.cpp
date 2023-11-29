#include "mlx/data/op/SampleTransform.h"

namespace mlx {
namespace data {
namespace op {

SampleTransform::SampleTransform(std::function<Sample(const Sample&)> op)
    : op_(op) {}

Sample SampleTransform::apply(const Sample& sample) const {
  return op_(sample);
}

} // namespace op
} // namespace data
} // namespace mlx
