#ifndef TVP_FFMPEG_HPP_INCLUDED
#define TVP_FFMPEG_HPP_INCLUDED
#include <exception>
extern "C" {
  #include <libavformat/avformat.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/avutil.h>
  #include <libavutil/imgutils.h>
  #include <libswscale/swscale.h>
}

#define AV_CHECK(cond) if (int res = (cond); res < 0)
#define AV_CHECKED(cond) \
  AV_CHECK(cond) throw ::av::av_error(res);
#define AV_CHECKED_THEN(cond) AV_CHECK(cond) throw ::av::av_error(res); else

namespace av {
  class av_error final : public std::exception {
  public:
    using std::exception::exception;
    
    av_error(int av_errno) : m_err_code(av_errno) {
      av_strerror(m_err_code, m_msg_data, sizeof(m_msg_data));
    }
    
    const char* what() const noexcept override {
      return m_msg_data;
    }
    
    int m_err_code;
    char m_msg_data[AV_ERROR_MAX_STRING_SIZE];
  };
  
  template <class T>
  inline T* check_alloc(T* in) {
    if (in == nullptr)
      throw av::av_error(AVERROR(ENOMEM));
    return in;
  }
  
  inline void frame_realloc_buffers(AVFrame* frame, size_t width, size_t height, AVPixelFormat fmt = AV_PIX_FMT_NONE) {
    AVPixelFormat real_fmt = (fmt == AV_PIX_FMT_NONE)? (AVPixelFormat) frame->format : fmt;
    
    if (frame->width != width || frame->height != height || frame->format != real_fmt) {
      av_frame_unref(frame);
      
      frame->width = width;
      frame->height = height;
      frame->format = real_fmt;
      
      av_frame_get_buffer(frame, 0);
    }
  }
}

#endif