#include <stdexcept>

#include "mlx/data/stream/Stream.h"

namespace mlx {
namespace data {
namespace stream {

Sample Stream::next() const {
  throw std::runtime_error("Stream::next() NYI");
}

void Stream::reset() {
  throw std::runtime_error("Stream::reset() NYI");
}

Stream::~Stream() {}

} // namespace stream
} // namespace data
} // namespace mlx
