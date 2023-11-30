// Copyright © 2023 Apple Inc.

#include "mlx/data/Array.h"

#include <istream>

namespace mlx {
namespace data {
namespace core {

struct membuf : std::streambuf {
  membuf(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};
struct imemstream : virtual membuf, std::istream {
  imemstream(std::shared_ptr<const mlx::data::Array> array)
      : membuf(static_cast<char*>(array->data()), array->size()),
        std::istream(static_cast<std::streambuf*>(this)),
        array_(array) {}
  std::shared_ptr<const mlx::data::Array> array_;
};

} // namespace core
} // namespace data
} // namespace mlx
