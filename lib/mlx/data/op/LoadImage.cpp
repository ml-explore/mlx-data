// Copyright © 2023 Apple Inc.

#include <filesystem>

#include "mlx/data/core/image/Image.h"
#include "mlx/data/op/LoadImage.h"

namespace mlx {
namespace data {
namespace op {
LoadImage::LoadImage(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    const std::string& format,
    bool from_memory,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      prefix_(prefix),
      info_(info),
      format_(format),
      from_memory_(from_memory) {}
std::shared_ptr<Array> LoadImage::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::filesystem::path path;
  std::shared_ptr<Array> dst;
  if (!from_memory_) {
    path = prefix_;
    if (src->type() != ArrayType::Int8) {
      throw std::runtime_error("LoadImage: char array (int8) expected");
    }
    std::string filename(reinterpret_cast<char*>(src->data()), src->size());
    path /= filename;
  }
  if (info_) {
    auto info = from_memory_ ? core::image::info(src) : core::image::info(path);
    std::vector<int64_t> info_array({info.width, info.height});
    dst = std::make_shared<Array>(info_array);
  } else {
    dst = from_memory_ ? core::image::load(src) : core::image::load(path);
    if (!dst) {
      throw std::runtime_error(
          "LoadImage: unable to load image <" +
          (from_memory_ ? "stream" : path.string()) + ">");
    }
  }
  return dst;
}
} // namespace op
} // namespace data
} // namespace mlx
