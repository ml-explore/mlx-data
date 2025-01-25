// Copyright © 2023-2024 Apple Inc.

#include "mlx/data/Array.h"
#include "mlx/data/Sample.h"

namespace mlx {
namespace data {
namespace core {

std::pair<std::shared_ptr<Array>, std::shared_ptr<Array>> uniq(
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double pad);

std::pair<std::shared_ptr<Array>, std::shared_ptr<Array>> remove(
    const std::shared_ptr<Array> src,
    const std::shared_ptr<Array> src_length,
    int dim,
    double value,
    double pad);

std::shared_ptr<Array> replace(
    const std::shared_ptr<const Array>& src,
    const std::shared_ptr<const Array>& old,
    const std::shared_ptr<const Array>& replacement,
    int count);

Sample merge_batch(
    const std::vector<Sample>& samples,
    const std::unordered_map<std::string, double>& pad_values = {},
    const std::unordered_map<std::string, int>& batch_dims = {});

} // namespace core
} // namespace data
} // namespace mlx
