#include <strings.h>
#include <filesystem>
#include "mlx/data/core/image/ImagePrivate.h"

#ifdef MLX_HAS_JPEG
#include <jpeglib.h>
#endif

namespace mlx {
namespace data {
namespace core {
namespace image {

#if MLX_HAS_JPEG

const static JOCTET EOI_BUFFER[1] = {JPEG_EOI};
struct JpegMemSourceMgr {
  struct jpeg_source_mgr pub;
  const uint8_t* data;
  size_t len;
};

static void jpeg_mem_init_source(j_decompress_ptr cinfo) {}

static boolean jpeg_mem_fill_input_buffer(j_decompress_ptr cinfo) {
  JpegMemSourceMgr* src = (JpegMemSourceMgr*)cinfo->src;
  // We have buffered everything when setting up cinfo
  // So EOF
  src->pub.next_input_byte = EOI_BUFFER;
  src->pub.bytes_in_buffer = 1;
  return TRUE;
}
static void jpeg_mem_skip_input_data(j_decompress_ptr cinfo, long count) {
  JpegMemSourceMgr* src = (JpegMemSourceMgr*)cinfo->src;
  if (src->pub.bytes_in_buffer < count) { // skip all?
    src->pub.next_input_byte = EOI_BUFFER;
    src->pub.bytes_in_buffer = 1;
  } else {
    src->pub.next_input_byte += count;
    src->pub.bytes_in_buffer -= count;
  }
}
static void jpeg_mem_term_source(j_decompress_ptr cinfo) {}

static void jpeg_mem_set_source_mgr(
    j_decompress_ptr cinfo,
    const uint8_t* data,
    size_t len) {
  if (cinfo->src == nullptr) {
    cinfo->src = (struct jpeg_source_mgr*)(*cinfo->mem->alloc_small)(
        (j_common_ptr)cinfo, JPOOL_PERMANENT, sizeof(JpegMemSourceMgr));
  }
  JpegMemSourceMgr* src = (JpegMemSourceMgr*)cinfo->src;
  src->pub.init_source = jpeg_mem_init_source;
  src->pub.fill_input_buffer = jpeg_mem_fill_input_buffer;
  src->pub.skip_input_data = jpeg_mem_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart;
  src->pub.term_source = jpeg_mem_term_source;
  // "buffer" all data at once
  src->data = (const JOCTET*)data;
  src->len = len;
  src->pub.bytes_in_buffer = len;
  src->pub.next_input_byte = src->data;
}

static bool verify_signature(uint8_t signature[3], int64_t signature_size) {
  static const uint8_t jpeg_signature[3] = {0xff, 0xd8, 0xff};
  return (signature_size == 3) &&
      (std::memcmp(jpeg_signature, signature, 3) == 0);
}

static bool check_signature(const std::string& path) {
  uint8_t signature[3];
  int64_t signature_size = 0;

  FILE* f = fopen(path.c_str(), "rb");
  if (!f) {
    throw std::runtime_error("load_jpeg: could not load <" + path + ">");
  }
  signature_size = fread(&signature, 1, 3, f);
  fclose(f);

  return verify_signature(signature, signature_size);
}

static bool check_signature(const std::shared_ptr<const Array> contents) {
  uint8_t signature[3];
  int64_t signature_size = 0;

  if (contents->size() >= 3) {
    memcpy(signature, contents->data(), 3);
    signature_size = 3;
  }
  return verify_signature(signature, signature_size);
}

static std::shared_ptr<Array> load_jpeg(struct jpeg_decompress_struct& info) {
  jpeg_read_header(&info, 1);
  jpeg_start_decompress(&info);
  int64_t w = info.output_width;
  int64_t h = info.output_height;
  int64_t c = 3; // info.num_components;
  auto result = std::make_shared<Array>(ArrayType::UInt8, h, w, c);
  if (info.num_components == 3) {
    while (info.output_scanline < info.output_height) {
      unsigned char* p = (unsigned char*)(result->data()) +
          3 * info.output_width * info.output_scanline;
      jpeg_read_scanlines(&info, &p, 1);
    }
  } else if (info.num_components == 4) {
    std::vector<unsigned char> buffer(4 * info.output_width);
    while (info.output_scanline < info.output_height) {
      auto p = buffer.data();
      unsigned char* p_dst = (unsigned char*)(result->data()) +
          3 * info.output_width * info.output_scanline;
      jpeg_read_scanlines(&info, &p, 1);
      for (int64_t i = 0; i < info.output_width; i++) {
        p_dst[i * 3] = p[i * 4];
        p_dst[i * 3 + 1] = p[i * 4 + 1];
        p_dst[i * 3 + 2] = p[i * 4 + 2];
      }
    }
  } else if (info.num_components == 1) {
    std::vector<unsigned char> buffer(1 * info.output_width);
    while (info.output_scanline < info.output_height) {
      auto p = buffer.data();
      unsigned char* p_dst = (unsigned char*)(result->data()) +
          3 * info.output_width * info.output_scanline;
      jpeg_read_scanlines(&info, &p, 1);
      for (int64_t i = 0; i < info.output_width; i++) {
        p_dst[i * 3] = p[i];
        p_dst[i * 3 + 1] = p[i];
        p_dst[i * 3 + 2] = p[i];
      }
    }
  } else {
    jpeg_destroy_decompress(&info);
    return nullptr;
  }
  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);

  return result;
}

std::shared_ptr<Array> load_jpeg(const std::string& path) {
  if (!check_signature(path)) {
    // not a jpeg
    return nullptr;
  }

  FILE* f = fopen(path.c_str(), "rb");
  if (!f) {
    throw std::runtime_error("load_jpeg: could not load <" + path + ">");
  }

  struct jpeg_decompress_struct info;
  struct CustomErrorMgr {
    struct jpeg_error_mgr err;
    FILE* f;
    const std::string* filename;
    jpeg_decompress_struct* info;
  } cerr;
  cerr.f = f;
  cerr.filename = &path;
  cerr.info = &info;

  info.err = jpeg_std_error((jpeg_error_mgr*)&cerr);
  cerr.err.error_exit = [](j_common_ptr cinfo) {
    char jpeg_last_error_msg[JMSG_LENGTH_MAX];
    (*(cinfo->err->format_message))(cinfo, jpeg_last_error_msg);
    struct CustomErrorMgr* cerr = (struct CustomErrorMgr*)(cinfo->err);
    if (cerr->f) {
      fclose(cerr->f);
    }
    jpeg_destroy_decompress(cerr->info);
    throw std::runtime_error(
        "load_jpeg: could not load <" + *cerr->filename + "> (" +
        std::string(jpeg_last_error_msg) + ")");
  };
  jpeg_create_decompress(&info);
  jpeg_stdio_src(&info, f);

  auto result = load_jpeg(info);
  fclose(f);

  if (result == nullptr) {
    throw std::runtime_error(
        "load_jpeg: could not load <" + path + "> (unhandled format)");
  }

  return result;
}

std::shared_ptr<Array> load_jpeg(const std::shared_ptr<const Array> contents) {
  if (!check_signature(contents)) {
    // not a jpeg
    return nullptr;
  }

  struct jpeg_decompress_struct info;
  struct CustomErrorMgr {
    struct jpeg_error_mgr err;
    jpeg_decompress_struct* info;
  } cerr;
  cerr.info = &info;

  info.err = jpeg_std_error((jpeg_error_mgr*)&cerr);
  cerr.err.error_exit = [](j_common_ptr cinfo) {
    char jpeg_last_error_msg[JMSG_LENGTH_MAX];
    (*(cinfo->err->format_message))(cinfo, jpeg_last_error_msg);
    struct CustomErrorMgr* cerr = (struct CustomErrorMgr*)(cinfo->err);
    jpeg_destroy_decompress(cerr->info);
    throw std::runtime_error(
        "load_jpeg: could not load from memory (" +
        std::string(jpeg_last_error_msg) + ")");
  };
  jpeg_create_decompress(&info);
  jpeg_mem_set_source_mgr(
      &info, (const uint8_t*)contents->data(), contents->size());

  auto result = load_jpeg(info);

  if (result == nullptr) {
    throw std::runtime_error(
        "load_jpeg: could not load from memory (unhandled format)");
  }

  return result;
}

bool save_jpeg(
    const std::shared_ptr<const Array> image,
    const std::string& path) {
  if (image->shape().size() != 3) {
    return false;
  }

  std::filesystem::path p = path;
  if (strcasecmp(p.extension().c_str(), ".jpg") != 0 &&
      strcasecmp(p.extension().c_str(), ".jpeg") != 0) {
    return false;
  }

  FILE* f = fopen(p.c_str(), "wb");

  if (!f) {
    throw std::runtime_error(
        "save_jpeg: could not open <" + path + "> for writing");
  }

  struct jpeg_compress_struct info;
  struct CustomErrorMgr {
    struct jpeg_error_mgr err;
    FILE* f;
    const std::string* filename;
    jpeg_compress_struct* info;
  } cerr;
  cerr.f = f;
  cerr.filename = &path;
  cerr.info = &info;

  info.err = jpeg_std_error((jpeg_error_mgr*)&cerr);
  cerr.err.error_exit = [](j_common_ptr cinfo) {
    char jpeg_last_error_msg[JMSG_LENGTH_MAX];
    (*(cinfo->err->format_message))(cinfo, jpeg_last_error_msg);
    struct CustomErrorMgr* cerr = (struct CustomErrorMgr*)(cinfo->err);
    if (cerr->f) {
      fclose(cerr->f);
    }
    jpeg_destroy_compress(cerr->info);
    throw std::runtime_error(
        "save_jpeg: could not write <" + *cerr->filename + "> (" +
        std::string(jpeg_last_error_msg) + ")");
  };
  jpeg_create_compress(&info);

  jpeg_stdio_dest(&info, f);

  auto shape = image->shape();

  info.image_width = (uint32_t)shape[1];
  info.image_height = (uint32_t)shape[0];
  info.input_components = (uint32_t)shape[2];
  if (info.input_components == 3) {
    info.in_color_space = JCS_RGB;
  } else if (info.input_components == 1) {
    info.in_color_space = JCS_GRAYSCALE;
  } else {
    throw std::runtime_error(
        "save_jpeg: unsupported color depth (" +
        std::to_string(info.input_components) + ")");
  }
  jpeg_set_defaults(&info);
  jpeg_set_quality(&info, 80, true);

  jpeg_start_compress(&info, true);

  JSAMPROW row_pointer[1];

  // assumes packed pixels and rows
  size_t bytes_per_row = info.image_width * info.input_components;
  while (info.next_scanline < info.image_height) {
    row_pointer[0] = static_cast<unsigned char*>(image->data()) +
        bytes_per_row * info.next_scanline;
    jpeg_write_scanlines(&info, row_pointer, 1);
  }

  jpeg_finish_compress(&info);
  jpeg_destroy_compress(&info);

  fclose(f);

  return true;
}

#else

std::shared_ptr<Array> load_jpeg(const std::string& path) {
  return nullptr;
}

std::shared_ptr<Array> load_jpeg(const std::shared_ptr<const Array> contents) {
  return nullptr;
}

bool save_jpeg(
    const std::shared_ptr<const Array> image,
    const std::string& path) {
  return false;
}

#endif

} // namespace image
} // namespace core
} // namespace data
} // namespace mlx
