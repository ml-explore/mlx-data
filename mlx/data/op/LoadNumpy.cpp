// Copyright © 2023 Apple Inc.

#include "mlx/data/op/LoadNumpy.h"
#include "mlx/data/core/Numpy.h"
#include "mlx/data/core/imemstream.h"

#include <filesystem>

namespace mlx {
namespace data {
namespace op {
LoadNumpy::LoadNumpy(
    const std::string& ikey,
    const std::string& prefix,
    bool from_memory,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), prefix_(prefix), from_memory_(from_memory) {}

std::shared_ptr<Array> LoadNumpy::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::shared_ptr<Array> dst;
  if (from_memory_) {
    auto stream = core::imemstream(src);
    dst = core::load_numpy(stream, "<stream>");
  } else {
    std::filesystem::path path = prefix_;
    if (src->type() != ArrayType::Int8) {
      throw std::runtime_error("LoadNumpy: char array (int8) expected");
    }
    std::string filename(reinterpret_cast<char*>(src->data()), src->size());
    path /= filename;
    dst = core::load_numpy(path);
  }
  return dst;
}
} // namespace op
} // namespace data
} // namespace mlx
