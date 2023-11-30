// Copyright © 2023 Apple Inc.

#include "mlx/data/core/video/VideoPrivate.h"

namespace mlx {
namespace data {
namespace core {
namespace video {

static std::shared_ptr<Array> load(VideoReader& reader) {
  VideoInfo video_info = reader.info();

  // allocate storage for the full video
  auto result = std::make_shared<Array>(
      ArrayType::UInt8,
      video_info.frames,
      video_info.height,
      video_info.width,
      3);

  for (int frame_number = 0; frame_number < video_info.frames; frame_number++) {
    auto frame = reader.read_frame(array::slice(result, frame_number));

    if (result == nullptr) {
      // finished early -- metadata does not match actual
      // number of frames, make a new buffer to fit the data

      auto new_shape = result->shape();
      new_shape[0] = frame_number;
      auto new_frames = std::make_shared<Array>(
          ArrayType::UInt8,
          frame_number,
          video_info.height,
          video_info.width,
          3);
      memcpy(
          new_frames->data(),
          result->data(),
          new_frames->itemsize() * new_frames->size());
      result = new_frames;

      break;
    }
  }

  return result;
}

std::shared_ptr<Array> load(const std::string& path) {
  VideoReader reader = VideoReader(path);
  return load(reader);
}

std::shared_ptr<Array> load(const std::shared_ptr<const Array>& contents) {
  VideoReader reader = VideoReader(contents);
  return load(reader);
}

VideoInfo info(const std::string& path) {
  VideoReader reader = VideoReader(path);
  return reader.info();
}

VideoInfo info(const std::shared_ptr<const Array>& contents) {
  VideoReader reader = VideoReader(contents);
  return reader.info();
}

void verify_video(const std::shared_ptr<const Array>& video) {
  auto dimensions = video->shape().size();
  if (dimensions != 4) {
    throw std::runtime_error(
        "verifyVideo: video must be 4 dimension Array (FHWC)");
  }

  if (channels(video) == 0 || channels(video) > 4) {
    throw std::runtime_error("verifyVideo: channels must be 0 <= c <= 4");
  }
}

} // namespace video
} // namespace core
} // namespace data
} // namespace mlx
