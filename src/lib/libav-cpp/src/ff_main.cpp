#include <fmt/core.h>


#include <memory>
#include <stdexcept>

extern "C" {
#include <libavcodec/codec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

void ffmpeg_check(int x, bool exact = false) {
  if ((x < 0) | ((x > 0) & exact)) {
    throw std::runtime_error(fmt::format("libav error: {}", x));
  }
}

template <>
struct fmt::formatter<AVRational> {
  constexpr auto parse(fmt::format_parse_context ctx) -> decltype(ctx.begin()) {
    auto it = ctx.begin();
    if (it != ctx.end() && *it != '}') {
      throw std::runtime_error("FU");
    }
    return it;
  }

  template <class FC>
  auto format(const AVRational& r, FC& ctx) {
    return fmt::format_to(ctx.out(), "{}/{}", r.num, r.den);
  }
};

int main(int argc, char* argv[]) {
  fmt::print("FFmpeg test\n");

  // Load input file
  AVFormatContext* fmt_context = NULL;
  ffmpeg_check(avformat_open_input(&fmt_context, argv[1], NULL, NULL), true);

  fmt::print("\e[1mContainer\e[0m\n");
  fmt::print("Format: {}\n", fmt_context->iformat->long_name);
  fmt::print("Length: {}\n", fmt_context->duration);
  fmt::print("Bitrate: {}\n\n", fmt_context->bit_rate);

  // Find stream info
  ffmpeg_check(avformat_find_stream_info(fmt_context, NULL));

  // Codec info
  for (size_t i = 0; i < fmt_context->nb_streams; i++) {
    AVStream* s                           = fmt_context->streams[i];
    AVCodecParameters* local_codec_params = s->codecpar;
    fmt::print("\e[1mStream {}\e[0m\n", i);
    fmt::print("Time base: {}\n", s->time_base);
    fmt::print("Base framerate: {}\n", s->r_frame_rate);
    fmt::print("Start time: {}\n", s->start_time);
    fmt::print("Duration: {}\n", s->duration);

    AVCodec* codec = avcodec_find_decoder(s->codecpar->codec_id);
    if (codec == nullptr) {
      fmt::print("Type: (unknown)\n");
      fmt::print("Codec: (unknown)\n");
    }
    else {
      const char* codec_type_str = av_get_media_type_string(codec->type);
      fmt::print("Type: {}\n", (codec_type_str == nullptr)? "(unknown)" : codec_type_str);
      fmt::print("Codec: {}\n", codec->name);
    }
  }

  // CLEANUP
  avformat_close_input(&fmt_context);
}