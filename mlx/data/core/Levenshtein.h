// Copyright © 2023 Apple Inc.

#include "mlx/data/Array.h"

namespace mlx {
namespace data {
namespace core {

std::shared_ptr<Array> levenshtein(
    const std::shared_ptr<Array> arr1,
    const std::shared_ptr<Array> len1,
    const std::shared_ptr<Array> arr2,
    const std::shared_ptr<Array> len2);

}
} // namespace data
} // namespace mlx
