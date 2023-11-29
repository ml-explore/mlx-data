#include <filesystem>
#include <sstream>

#include "mlx/data/core/image/Image.h"
#include "mlx/data/op/SaveImage.h"

namespace mlx {
namespace data {
namespace op {
SaveImage::SaveImage(
    const std::string& image_key,
    const std::string& filename_key,
    const std::string& prefix,
    const std::string& filename_prefix)
    : imageKey_(image_key),
      filenameKey_(filename_key),
      prefix_(prefix),
      filenamePrefix_(filename_prefix) {}
Sample SaveImage::apply(const Sample& sample) const {
  std::shared_ptr<Array> input_array;
  input_array = sample::check_key(sample, imageKey_, ArrayType::UInt8);

  std::filesystem::path path = prefix_;

  std::shared_ptr<Array> base_filename_array;
  base_filename_array =
      sample::check_key(sample, filenameKey_, ArrayType::Int8);
  std::string base_filename(
      reinterpret_cast<char*>(base_filename_array->data()),
      base_filename_array->size());

  if (filenamePrefix_.length() > 0) {
    path /= filenamePrefix_ + base_filename;
  } else {
    path /= base_filename;
  }

  auto shape = input_array->shape();
  if (shape.size() == 4) {
    // a vector of images (e.g. a video)
    for (int i = 0; i < shape[0]; i++) {
      auto frame = array::slice(input_array, i);

      std::stringstream ext;
      ext << std::setw(6) << std::setfill('0') << i << ".jpg";

      auto frame_path = path;
      frame_path.replace_extension(ext.str());

      if (!core::image::save(frame, frame_path)) {
        throw std::runtime_error(
            "SaveImage: unable to save frame " + frame_path.string());
      }
    }
  } else {
    // simple image: HxWxC
    path.replace_extension("jpg");
    if (!core::image::save(input_array, path)) {
      throw std::runtime_error(
          "SaveImage: no provider to save image " + path.string());
    }
  }

  return sample;
}
} // namespace op
} // namespace data
} // namespace mlx
