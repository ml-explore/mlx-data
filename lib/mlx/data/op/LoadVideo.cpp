#include <filesystem>

#include "mlx/data/core/video/Video.h"
#include "mlx/data/op/LoadVideo.h"

namespace mlx {
namespace data {
namespace op {
LoadVideo::LoadVideo(
    const std::string& ikey,
    const std::string& prefix,
    bool info,
    bool from_memory,
    const std::string& okey)
    : KeyTransformOp(ikey, okey),
      prefix_(prefix),
      info_(info),
      from_memory_(from_memory) {}
std::shared_ptr<Array> LoadVideo::apply_key(
    const std::shared_ptr<const Array>& src) const {
  std::filesystem::path path;

  if (!from_memory_) {
    path = prefix_;
    if (src->type() != ArrayType::Int8) {
      throw std::runtime_error("LoadImage: char array (int8) expected");
    }
    std::string filename(reinterpret_cast<char*>(src->data()), src->size());
    path /= filename;
  }

  std::shared_ptr<Array> dst;
  if (info_) {
    auto info = from_memory_ ? core::video::info(src) : core::video::info(path);

    std::vector<int64_t> shape({info.width, info.height, info.frames});
    dst = std::make_shared<Array>(shape);

  } else {
    dst = from_memory_ ? core::video::load(src) : core::video::load(path);
    if (!dst) {
      throw std::runtime_error(
          "LoadVideo: unable to load video <" +
          (from_memory_ ? "stream" : path.string()) + ">");
    }
  }

  return dst;
}
} // namespace op
} // namespace data
} // namespace mlx
