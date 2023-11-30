// Copyright © 2023 Apple Inc.

#pragma once

#include "mlx/data/core/image/Image.h"

namespace mlx {
namespace data {
namespace core {
namespace image {

/// implementation for mlx::data::image

std::shared_ptr<Array> load_stbi(const std::string& path);
std::shared_ptr<Array> load_stbi(const std::shared_ptr<const Array> contents);

ImageInfo info_stbi(const std::string& path);
ImageInfo info_stbi(const std::shared_ptr<const Array> contents);

std::shared_ptr<Array> load_jpeg(const std::string& path);
std::shared_ptr<Array> load_jpeg(const std::shared_ptr<const Array> contents);

bool save_jpeg(
    const std::shared_ptr<const Array> image,
    const std::string& path);

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
