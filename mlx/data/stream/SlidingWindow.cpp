// Copyright © 2023 Apple Inc.

#include <iterator>

#include "mlx/data/stream/SlidingWindow.h"

namespace mlx {
namespace data {
namespace stream {
SlidingWindow::SlidingWindow(
    const std::shared_ptr<Stream>& stream,
    const std::string& key,
    int64_t size,
    int64_t stride,
    int dim,
    const std::string& index_key)
    : stream_(stream),
      key_(key),
      size_(size),
      stride_(stride),
      dim_(dim),
      index_key_(index_key) {
  if (size <= 0) {
    throw std::runtime_error("SlidingWindow: size must be strictly positive");
  }
  if (stride <= 0) {
    throw std::runtime_error("SlidingWindow: stride must be strictly positive");
  }
}

Sample SlidingWindow::next() const {
  std::unique_lock lock(mutex_);

  // Check if we already created some samples in which case simply return
  // the first one.
  if (!buffer_.empty()) {
    auto sample = std::move(buffer_.front());
    buffer_.pop();
    return sample;
  }

  // The buffer is empty so we need to get the next sample from the stream
  // and make a sliding window that is saved in the buffer_.
  std::queue<Sample> buffer;
  while (buffer.empty()) {
    // Fetch the next full sample
    auto sample = stream_->next();
    if (sample.empty()) {
      return sample;
    }

    auto array = sample::check_key(sample, key_, ArrayType::Any);
    int dim = array->checkdim(dim_);
    int64_t length = array->shape(dim);
    auto newshape = array->shape();
    std::vector<int64_t> newoffset(array->ndim(), 0);
    int64_t offset = 0;
    int64_t slice_index = 0;
    while (offset < length) {
      auto newsample = sample;
      int64_t newlength =
          ((size_ <= (length - offset)) ? size_ : (length - offset));
      newshape[dim] = newlength;
      newoffset[dim] = offset;
      newsample[key_] = array::sub(array, newoffset, newshape);
      if (!index_key_.empty()) {
        newsample[index_key_] = std::make_shared<Array>(slice_index);
      }
      buffer.emplace(newsample);
      offset += stride_;
      slice_index++;
    }
  }

  auto sample = std::move(buffer.front());
  buffer.pop();
  buffer_ = buffer;

  return sample;
}

void SlidingWindow::reset() {
  std::unique_lock lock(mutex_);
  buffer_ = std::queue<Sample>();
  stream_->reset();
}

} // namespace stream
} // namespace data
} // namespace mlx
