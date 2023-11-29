#include "mlx/data/buffer/Batch.h"
#include "mlx/data/core/Utils.h"

namespace mlx {
namespace data {
namespace buffer {
Batch::Batch(
    const std::shared_ptr<Buffer>& op,
    int64_t batch_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : op_(op),
      batchSize_(batch_size),
      padValues_(pad_values),
      batchDims_(batch_dims) {
  if (batch_size <= 0) {
    throw std::runtime_error("Batch: batch size must be positive");
  }
  size_ = op->size() / batch_size;
  if (op->size() % batch_size) {
    size_++;
  }
}
Batch::Batch(
    const std::shared_ptr<Buffer>& op,
    const std::vector<int64_t>& batch_sizes,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims)
    : op_(op),
      batchSize_(0),
      batchOffsets_(batch_sizes.size()),
      batchSizes_(batch_sizes),
      padValues_(pad_values),
      batchDims_(batch_dims) {
  int64_t batch_sizes_sum = 0;
  for (int64_t i = 0; i < batch_sizes.size(); i++) {
    auto batch_size = batch_sizes[i];
    if (batch_size <= 0) {
      throw std::runtime_error("Batch: batch size must be positive");
    }
    batchOffsets_[i] = batch_sizes_sum;
    batch_sizes_sum += batch_size;
  }
  if (batch_sizes_sum > op->size()) {
    throw std::runtime_error("Batch: sum of batch sizes exceeds buffer size");
  }
  size_ = batch_sizes.size();
}

Sample Batch::get(int64_t idx) const {
  if (idx < 0 || idx >= size_) {
    throw std::runtime_error("Batch: index out of range");
  }
  auto batch_size =
      (batchSize_ ? std::min(batchSize_, op_->size() - idx * batchSize_)
                  : batchSizes_[idx]);
  auto batch_offset = (batchSize_ ? idx * batchSize_ : batchOffsets_[idx]);
  std::vector<Sample> samples(batch_size);
  for (int64_t i = 0; i < batch_size; i++) {
    samples[i] = op_->get(batch_offset + i);
  }
  return core::merge_batch(samples, padValues_, batchDims_);
}

int64_t Batch::size() const {
  return size_;
}

} // namespace buffer
} // namespace data
} // namespace mlx
