#include "mlx/data/op/LoadFile.h"

#include <fstream>

namespace mlx {
namespace data {
namespace op {
LoadFile::LoadFile(
    const std::string& ikey,
    const std::filesystem::path& prefix,
    const std::string& okey)
    : KeyTransformOp(ikey, okey), prefix_(prefix) {}

std::shared_ptr<Array> LoadFile::apply_key(
    const std::shared_ptr<const Array>& src) const {
  if (src->type() != ArrayType::Int8) {
    throw std::runtime_error("LoadFile: char array (int8) expected");
  }
  std::filesystem::path path = prefix_;
  std::string filename(reinterpret_cast<char*>(src->data()), src->size());
  path /= filename;

  std::shared_ptr<Array> dst;
  std::ifstream file;
  file.exceptions(std::ifstream::badbit);
  try {
    file.open(path, std::ios::binary | std::ios::ate);
    int64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    dst = std::make_shared<Array>(Int8, file_size);
    file.read(dst->data<char>(), file_size);
    file.close();
  } catch (const std::ifstream::failure& e) {
    throw std::runtime_error(
        std::string("LoadFile: unable to read ") + path.string());
  }
  return dst;
}
} // namespace op
} // namespace data
} // namespace mlx
