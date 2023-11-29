#include "mlx/data/core/video/VideoPrivate.h"

#include <string>

#if MLX_HAS_FFMPEG

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

#include <libavformat/version.h>
}

#endif

namespace mlx {
namespace data {
namespace core {
namespace video {

#if MLX_HAS_FFMPEG

/// callback from the AVCodecContext to select the pixelFormat
static enum AVPixelFormat select_hardware_pixel_format(
    AVCodecContext* context,
    const enum AVPixelFormat* pixel_formats) {
  const enum AVPixelFormat* p;

  // prefer an RGB pixel format
  for (p = pixel_formats; *p != -1; p++) {
    switch (*p) {
      case AV_PIX_FMT_RGB24:
      case AV_PIX_FMT_BGR24:
      case AV_PIX_FMT_ARGB:
      case AV_PIX_FMT_RGBA:
      case AV_PIX_FMT_ABGR:
      case AV_PIX_FMT_BGRA:
        return *p;
      default:
        break;
    }
  }

  // otherwise fall back to the first available
  return pixel_formats[0];
}

static void
check_av_error(const std::string message, const std::string filename, int ret) {
  if (ret < 0) {
    char error[256];
    av_make_error_string(error, 256, ret);

    throw std::runtime_error(
        std::string("VideoReader error ") + message + " <" + filename +
        ">: " + std::string(error));
  }
}

static bool should_use_hw_codec(
    AVCodecContext* codec_context,
    AVStream* stream) {
  // do not use the hw decoder for small videos -- 96x96 is 3 to 10x slower
  // using hardware over sw.
  int threshold = 720 * 1080;
  int pixels = codec_context->width * codec_context->height;
  return pixels >= threshold;
}

static AVHWDeviceType find_hwdevice_type() {
  // find a preferred hardware accelerator type -- these are not in any
  // order and in particular vdpau requires an X11 session to actually work.
  enum AVHWDeviceType type = av_hwdevice_iterate_types(AV_HWDEVICE_TYPE_NONE);

  while (type != AV_HWDEVICE_TYPE_NONE) {
    if (type == AV_HWDEVICE_TYPE_VIDEOTOOLBOX) {
      return type;
    }
    if (type == AV_HWDEVICE_TYPE_CUDA) {
      return type;
    }
#ifdef AV_HWDEVICE_TYPE_VULKAN
    if (type == AV_HWDEVICE_TYPE_VULKAN) {
      return type;
    }
#endif

    type = av_hwdevice_iterate_types(type);
  }

  return AV_HWDEVICE_TYPE_NONE;
}

/// Opaque state of the public `VideoReader`.
class VideoReaderState {
 public:
  VideoReaderState(const std::string& filename);
  VideoReaderState(const std::shared_ptr<const Array>& contents);

  ~VideoReaderState();

  VideoInfo info();
  std::shared_ptr<Array> read_frame(std::shared_ptr<Array> destination);

  std::string filename_;

  /// state for stream based readers.  This is referenced via an AVIOContext
  /// and a callback (see `streamReadFunction()`).
  std::shared_ptr<const Array> streamContents_;
  size_t streamPosition_;
  size_t streamLength_;

  /// represents the file
  AVFormatContext* formatContext_;

  /// represents the decoder
  AVCodecContext* decoderContext_;

  /// optional hardware context if we are able to use hardware acceleration
  AVBufferRef* hardwareDeviceContext_;

  /// the stream we are reading (pointer to `formatContext_->streams[i]`)
  AVStream* videoStream_;

  void init(std::string filename);
};

/// RAII container for reading a single frame and managing the state associated
/// with that
class FrameReader {
 public:
  FrameReader();
  ~FrameReader();

  std::shared_ptr<Array> read_frame(
      std::string filename,
      std::shared_ptr<Array> destination,
      AVFormatContext* format_context,
      AVCodecContext* decoder_context,
      AVStream* stream);

 private:
  /// the input packet (bytes from the input file)
  AVPacket* packet_;

  /// the frame from the decoder (may be on the gpu/hw)
  AVFrame* frame_;

  /// optional, the frame in main memory
  AVFrame* cpuFrame_;

  /// context used to do format translation, e.g. YUV420 to RGB
  SwsContext* scalerContext_;

  enum State { FRAME_READER_READING, FRAME_READER_FLUSHING, FRAME_READER_EOF };

  State state_;

  std::shared_ptr<Array> decode_(
      std::string filename,
      std::shared_ptr<Array> destination,
      AVCodecContext* decoder_context);
};

VideoReaderState::VideoReaderState(const std::string& filename)
    : filename_(filename) {
  int ret;

  formatContext_ = nullptr;

  // open the file and parse the structure
  ret = avformat_open_input(&formatContext_, filename.c_str(), NULL, NULL);
  check_av_error("opening file", filename, ret);

  init(filename);
}

/// callback from AVIOContext
static int stream_read_function(void* opaque, uint8_t* buf, int buf_size) {
  auto& state = *reinterpret_cast<VideoReaderState*>(opaque);

  size_t read_size;
  if (state.streamPosition_ + buf_size > state.streamLength_) {
    read_size = state.streamLength_ - state.streamPosition_;
  } else {
    read_size = buf_size;
  }

  unsigned char* src =
      (unsigned char*)state.streamContents_->data() + state.streamPosition_;

  memcpy(buf, src, read_size);

  state.streamPosition_ += read_size;
  return (int)read_size;
}

/// callback from AVIOContext
static int64_t stream_seek_function(void* opaque, int64_t offset, int whence) {
  auto& state = *reinterpret_cast<VideoReaderState*>(opaque);
  switch (whence) {
    case AVSEEK_SIZE:
      return state.streamLength_;
    case AVSEEK_FORCE:
      state.streamPosition_ = offset;
      break;
    case SEEK_SET:
      state.streamPosition_ = offset;
      break;
    case SEEK_CUR:
      state.streamPosition_ += offset;
      break;
    case SEEK_END:
      state.streamPosition_ = state.streamLength_ + offset;
      break;
  }

  return state.streamPosition_;
}

VideoReaderState::VideoReaderState(
    const std::shared_ptr<const Array>& contents) {
  int ret;

  // set up the stream state
  this->streamContents_ = contents;
  streamPosition_ = 0;
  streamLength_ = contents->size() * contents->itemsize();

  // this is the context that holds the state and callbacks
  const int buffer_size = 8192;
  AVIOContext* io_context = avio_alloc_context(
      (unsigned char*)av_malloc(buffer_size),
      buffer_size,
      0,
      reinterpret_cast<void*>(this),
      &stream_read_function,
      nullptr,
      &stream_seek_function);

  io_context->seekable = AVIO_SEEKABLE_NORMAL;

  // create the AVFormatContext (represents the file) and use our IOContext
  // to provide the contents from memory.
  formatContext_ = avformat_alloc_context();
  formatContext_->pb = io_context;
  formatContext_->flags |= AVFMT_FLAG_CUSTOM_IO;

  ret = avformat_open_input(&formatContext_, "<stream>", nullptr, nullptr);
  check_av_error("opening file", "<stream>", ret);

  init("<stream>");
}

/// finish initializing the `VideoReaderState`.  Requires that the
/// `formatContext_` be set.
void VideoReaderState::init(std::string filename) {
  int ret;

#if LIBAVFORMAT_VERSION_MAJOR <= 58
  // older versions of the library (apt-get) do not have the const
  AVCodec* decoder = nullptr;
#else
  AVCodec const* decoder = nullptr;
#endif

  ret = avformat_find_stream_info(formatContext_, NULL);
  check_av_error("avformat_find_stream_info", filename, ret);

  // find the best video stream and its associated codec
  int video_stream_index = av_find_best_stream(
      formatContext_, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
  check_av_error("av_find_best_stream", filename, video_stream_index);

  hardwareDeviceContext_ = nullptr;
  decoderContext_ = avcodec_alloc_context3(decoder);

  videoStream_ = formatContext_->streams[video_stream_index];

  // configure the hardware acceleration, if possible
  enum AVHWDeviceType type = should_use_hw_codec(decoderContext_, videoStream_)
      ? find_hwdevice_type()
      : AV_HWDEVICE_TYPE_NONE;

  if (type != AV_HWDEVICE_TYPE_NONE) {
    bool codec_supported = false;
    for (int i = 0;; i++) {
      const AVCodecHWConfig* config = avcodec_get_hw_config(decoder, i);
      if (!config) {
        // this codec is not supported by the hardware
        break;
      }
      if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
          config->device_type == type) {
        codec_supported = true;
        break;
      }
    }

    if (codec_supported) {
      ret =
          av_hwdevice_ctx_create(&hardwareDeviceContext_, type, NULL, NULL, 0);
      check_av_error("av_hwdevice_ctx_create", filename, ret);

      decoderContext_->hw_device_ctx = av_buffer_ref(hardwareDeviceContext_);

      // configure the callback to obtain the pixel format
      decoderContext_->get_format = select_hardware_pixel_format;
    }
  }

  ret = avcodec_parameters_to_context(decoderContext_, videoStream_->codecpar);
  check_av_error("avcodec_parameters_to_context", filename, ret);

  ret = avcodec_open2(decoderContext_, decoder, NULL);
  check_av_error("avcodec_open2", filename, ret);
}

VideoReaderState::~VideoReaderState() {
  avcodec_free_context(&decoderContext_);
  if (hardwareDeviceContext_) {
    av_buffer_unref(&hardwareDeviceContext_);
  }

  AVIOContext* io_context = formatContext_->pb;
  if (io_context && (formatContext_->flags & AVFMT_FLAG_CUSTOM_IO)) {
    // important: internally AVIOContext may re-allocate the buffer that
    // we initially supplied.  It is important that we free the current buffer.
    //
    // see:
    // https://lists.ffmpeg.org/pipermail/libav-user/2012-December/003257.html
    av_free(io_context->buffer);
    avio_context_free(&io_context);
  }

  avformat_close_input(&formatContext_);
}

VideoInfo VideoReaderState::info() {
  return {
      decoderContext_->width,
      decoderContext_->height,
      3,
      videoStream_->nb_frames};
}

std::shared_ptr<Array> VideoReaderState::read_frame(
    std::shared_ptr<Array> destination) {
  FrameReader reader;
  return reader.read_frame(
      filename_, destination, formatContext_, decoderContext_, videoStream_);
}

FrameReader::FrameReader() {
  state_ = FRAME_READER_READING;
  frame_ = av_frame_alloc();
  packet_ = av_packet_alloc();
  cpuFrame_ = nullptr;
  scalerContext_ = nullptr;
}

FrameReader::~FrameReader() {
  av_frame_free(&frame_);
  av_frame_free(&cpuFrame_);
  av_packet_free(&packet_);
  sws_freeContext(scalerContext_);
}

std::shared_ptr<Array> FrameReader::read_frame(
    std::string filename,
    std::shared_ptr<Array> destination,
    AVFormatContext* format_context,
    AVCodecContext* decoder_context,
    AVStream* stream) {
  // read packets until we hit the end of file or read a frame for our stream

  if (state_ == FRAME_READER_EOF) {
    return nullptr;
  }

  // see if there is anything left in the codec and return it
  auto result = decode_(filename, destination, decoder_context);
  if (result != nullptr) {
    return result;
  } else if (state_ == FRAME_READER_FLUSHING) {
    // we were flushing the remaining frames out and ran empty
    state_ = FRAME_READER_EOF;
  }

  // read packets from the input until 1) we get a packet for our target
  // stream and 2) that produces a frame
  while (true) {
    // note: all paths must call av_packet_unref once per
    // call to av_read_frame AND must call av_packet_free
    // (in the destructor)
    int ret = av_read_frame(format_context, packet_);

    if (ret == AVERROR_EOF) {
      // reached the end of the input file, now send a null packet
      // into the decoder to flush any remaining frames out.
      av_packet_unref(packet_);

      state_ = FRAME_READER_FLUSHING;
      ret = avcodec_send_packet(decoder_context, nullptr);
      check_av_error("avcodec_send_packet (eof)", filename, ret);

      auto result = decode_(filename, destination, decoder_context);

      if (result == nullptr) {
        state_ = FRAME_READER_EOF;
      }

      return result;
    }

    check_av_error("av_read_frame", filename, ret);

    if (packet_->stream_index == stream->index) {
      // nothing in the pipeline, send the packet and decode it
      ret = avcodec_send_packet(decoder_context, packet_);
      check_av_error("avcodec_send_packet", filename, ret);

      av_packet_unref(packet_);

      auto result = decode_(filename, destination, decoder_context);
      if (result != nullptr) {
        return result;
      }
    } else {
      av_packet_unref(packet_);
    }
  }
}

std::shared_ptr<Array> FrameReader::decode_(
    std::string filename,
    std::shared_ptr<Array> destination,
    AVCodecContext* decoder_context) {
  int ret;

  ret = avcodec_receive_frame(decoder_context, frame_);
  if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    return nullptr;
  }
  check_av_error("avcodec_receive_frame", filename, ret);

  AVFrame* frame_to_process = frame_;

  // if the frame has a hardware context it needs to be transferred from the gpu
  if (frame_->hw_frames_ctx) {
    cpuFrame_ = av_frame_alloc();
    frame_to_process = cpuFrame_;

    ret = av_hwframe_transfer_data(cpuFrame_, frame_, 0);
    check_av_error("av_hwframe_transfer_data", filename, ret);
  }

  auto result = destination == nullptr ? std::make_shared<Array>(
                                             ArrayType::UInt8,
                                             frame_to_process->height,
                                             frame_to_process->width,
                                             3)
                                       : destination;

  // convert from the video fram format, e.g. a planar YUV, into RGB24.
  scalerContext_ = sws_getContext(
      frame_to_process->width,
      frame_to_process->height,
      (enum AVPixelFormat)frame_to_process->format,
      frame_to_process->width,
      frame_to_process->height,
      AV_PIX_FMT_RGB24,
      (int)SWS_FAST_BILINEAR,
      nullptr,
      nullptr,
      nullptr);

  int dest_strides[4];
  ret = av_image_fill_linesizes(
      dest_strides, AV_PIX_FMT_RGB24, frame_to_process->width);
  check_av_error("av_image_fill_linesizes", filename, ret);

  uint8_t* dest_data[4] = {0};
  dest_data[0] = (uint8_t*)result->data();

  ret = sws_scale(
      scalerContext_,
      frame_to_process->data,
      frame_to_process->linesize,
      0,
      frame_to_process->height,
      dest_data,
      dest_strides);
  check_av_error("sws_scale", filename, ret);

  return result;
}

VideoReader::VideoReader(const std::string& filename) {
  state_ = new VideoReaderState(filename);
}

VideoReader::VideoReader(const std::shared_ptr<const Array>& stream) {
  state_ = new VideoReaderState(stream);
}

VideoReader::~VideoReader() {
  delete state_;
}

VideoInfo VideoReader::info() {
  return state_->info();
}

std::shared_ptr<Array> VideoReader::read_frame(
    std::shared_ptr<Array> destination) {
  return state_->read_frame(destination);
}

#else // MLX_HAS_FFMPEG

VideoReader::VideoReader(const std::string& filename) {
  state_ = nullptr; // avoids unused warning
}

VideoReader::VideoReader(const std::shared_ptr<const Array>& stream) {
  state_ = nullptr; // avoids unused warning
}

VideoReader::~VideoReader() {}

VideoInfo VideoReader::info() {
  throw std::runtime_error(
      std::string("VideoReader error: ffmpg libraries not found"));
}

std::shared_ptr<Array> VideoReader::read_frame(
    std::shared_ptr<Array> destination) {
  throw std::runtime_error(
      std::string("VideoReader error: ffmpg libraries not found"));
}

#endif

} // namespace video
} // namespace core
} // namespace data
} // namespace mlx
