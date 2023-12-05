// Copyright © 2023 Apple Inc.

#include <cstdint>
#include <vector>

namespace mlx {
namespace data {
namespace core {

// Computes the shape of a batch
class BatchShape {
 public:
  // batch by prefixing an extra dim
  BatchShape();

  // batch along a specified dim
  BatchShape(int dim);

  // add a shape to the batch
  void add(const std::vector<int64_t>& shape);

  void clear();

  int64_t size() const;
  const std::vector<int64_t>& shape() const;
  int64_t num_sample() const;
  int64_t operator[](int dim) const;

 private:
  std::vector<int64_t> shape_;
  int dim_;
  bool nodim_;
  int64_t num_sample_;
};
} // namespace core
} // namespace data
} // namespace mlx
