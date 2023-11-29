#include "mlx/data/stream/DynamicBatch.h"
#include "mlx/data/buffer/DynamicBatch.h"
#include "mlx/data/buffer/Shuffle.h"

namespace mlx {
namespace data {
namespace stream {

DynamicBatch::DynamicBatch(
    std::shared_ptr<Stream> stream,
    int64_t buffer_size,
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool shuffle,
    int num_thread)
    : Buffered(
          stream,
          buffer_size,
          onRefill_(key, max_data_size, pad_values, batch_dims, shuffle),
          num_thread){};

std::function<
    std::shared_ptr<buffer::Buffer>(const std::shared_ptr<buffer::Buffer>)>
DynamicBatch::onRefill_(
    const std::string& key,
    int64_t max_data_size,
    const std::unordered_map<std::string, double>& pad_values,
    const std::unordered_map<std::string, int>& batch_dims,
    bool shuffle) {
  auto on_refill = [key, max_data_size, pad_values, batch_dims, shuffle](
                       std::shared_ptr<buffer::Buffer> buffer) {
    buffer = std::make_shared<buffer::DynamicBatch>(
        buffer, key, max_data_size, pad_values, batch_dims);
    if (shuffle) {
      buffer = std::make_shared<buffer::Shuffle>(buffer);
    }
    return buffer;
  };

  return on_refill;
}

} // namespace stream
} // namespace data
} // namespace mlx
