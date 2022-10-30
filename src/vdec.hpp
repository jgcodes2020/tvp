#ifndef TVP_VDEC_HPP
#define TVP_VDEC_HPP

#include "ffmpeg.hpp"

#include <filesystem>

namespace tvp {
  class video_decoder {
  public:
    video_decoder(const std::filesystem::path& path);
    
    video_decoder(const video_decoder&) = delete;
    video_decoder& operator=(const video_decoder&) = delete;
    
    video_decoder(video_decoder&& that) : 
      m_format_ctx(that.m_format_ctx),
      m_codec_ctx(that.m_codec_ctx),
      m_codec(that.m_codec),
      m_packet(that.m_packet),
      m_frame(that.m_frame),
      m_index(that.m_index)
    {
      that.m_format_ctx = nullptr;
      that.m_codec_ctx = nullptr;
      that.m_packet = nullptr;
      that.m_frame = nullptr;
    }
    video_decoder& operator=(video_decoder&& that) {
      m_format_ctx = that.m_format_ctx;
      m_codec_ctx = that.m_codec_ctx;
      m_codec = that.m_codec;
      m_packet = that.m_packet;
      m_frame = that.m_frame;
      m_index = that.m_index;
      
      that.m_format_ctx = nullptr;
      that.m_codec_ctx = nullptr;
      that.m_packet = nullptr;
      that.m_frame = nullptr;
      
      return *this;
    }
    
    ~video_decoder() {
      if (m_format_ctx != nullptr) {
        avformat_free_context(m_format_ctx);
        m_format_ctx = nullptr;
      }
      if (m_codec_ctx != nullptr)
        avcodec_free_context(&m_codec_ctx);
      if (m_packet != nullptr)
        av_packet_free(&m_packet);
      if (m_frame != nullptr)
        av_frame_free(&m_frame);
    }
    
    // Returns the next frame, or nullptr on EOF.
    const AVFrame* next();
    
    // Current frame counter.
    size_t count() {
      return m_counter;
    }
  private:
    AVFormatContext* m_format_ctx;
    AVCodecContext* m_codec_ctx;
    const AVCodec* m_codec;
    AVPacket* m_packet;
    AVFrame* m_frame;
    size_t m_index;
    size_t m_counter;
  };
}
#endif