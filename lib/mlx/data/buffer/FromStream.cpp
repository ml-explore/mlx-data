#include "mlx/data/buffer/FromStream.h"

namespace mlx {
namespace data {
namespace buffer {

FromStream::FromStream(
    const std::shared_ptr<stream::Stream>& stream,
    int64_t size)
    : FromVector(bufferize_(stream, size)) {}

std::vector<Sample> FromStream::bufferize_(
    std::shared_ptr<stream::Stream> stream,
    int64_t size) {
  std::vector<Sample> buffer;
  if (size > 0) {
    buffer.reserve(size);
  }
  for (int64_t i = 0; (size < 0) || (i < size); i++) {
    auto sample = stream->next();
    if (sample.empty()) {
      break;
    }
    buffer.push_back(sample);
  }
  return buffer;
}

} // namespace buffer
} // namespace data
} // namespace mlx
