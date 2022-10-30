#include "vdec.hpp"
#include "ffmpeg.hpp"

namespace {
  thread_local char _errmsg[AV_ERROR_MAX_STRING_SIZE];
}

namespace tvp {
  video_decoder::video_decoder(const std::filesystem::path& path) :
    m_format_ctx(nullptr), m_counter(0) {
    
    // Open media file
    AV_CHECKED(avformat_open_input(
      &m_format_ctx, path.string().c_str(), nullptr, nullptr));
    AV_CHECKED(avformat_find_stream_info(m_format_ctx, nullptr));
    
    // Check if there is video
    AV_CHECKED_THEN(av_find_best_stream(
      m_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &m_codec, 0)) {
      m_index = res;
    }
    
    // Construct and open codec context
    m_codec_ctx = av::check_alloc(avcodec_alloc_context3(m_codec));
    AV_CHECKED(avcodec_parameters_to_context(
      m_codec_ctx, m_format_ctx->streams[m_index]->codecpar));
    AV_CHECKED(avcodec_open2(m_codec_ctx, m_codec, nullptr));
    
    // Construct the frame and packet
    m_packet = av::check_alloc(av_packet_alloc());
    m_frame = av::check_alloc(av_frame_alloc());
  }

  const AVFrame* video_decoder::next() {
    int res;
    
    m_counter++;
    // Clean up the previous frame
    if (m_frame->buf[0] != nullptr)
      av_frame_unref(m_frame);
    
    // Try reading a frame
    res = avcodec_receive_frame(m_codec_ctx, m_frame);
    if (res == 0)
      return m_frame;
    else if (res != AVERROR(EAGAIN))
      throw av::av_error(res);
    
    // Clean up the previous packet
    if (m_packet->data != nullptr)
      av_packet_unref(m_packet);
    
    // Read a packet
    res = av_read_frame(m_format_ctx, m_packet);
    if (res == 0) {
      AV_CHECKED(avcodec_send_packet(m_codec_ctx, m_packet));
    }
    else if (res == AVERROR_EOF) {
      // Flush the decoder
      res = avcodec_send_packet(m_codec_ctx, nullptr);
      // Flush returns EOF on hitting EOF
      if (res == AVERROR_EOF)
        return nullptr;
    }
    
    // Try reading a frame again
    AV_CHECKED_THEN(avcodec_receive_frame(m_codec_ctx, m_frame)) {
      return m_frame;
    }
  }
}  // namespace tvp