#include "vdec.hpp"
#include <fmt/color.h>
#include <libavutil/avutil.h>
#include "codec.h"
#include "codeccontext.h"
#include "formatcontext.h"
#include "frame.h"

#include <filesystem>
#include <ranges>
#include <stdexcept>

namespace fs = std::filesystem;

namespace tvp {
  struct video_decoder::impl {
    impl(const fs::path& path) : m_fmt(), m_vdec(), m_vs(), m_vs_idx() {
      m_fmt.openInput(path);
      
      m_fmt.findStreamInfo();
      
      // Find a video stream in the provided file
      for (size_t i = 0; i < m_fmt.streamsCount(); i++) {
        auto str = m_fmt.stream(i);
        if (str.isVideo() && str.isValid()) {
          m_vs_idx = i;
          m_vs = str;
          break;
        }
      }
      if (!m_vs.isValid()) {
        throw std::runtime_error("Provided file does not contain video data");
      }
      
      // Find the codec
      {
        m_vdec = av::VideoDecoderContext(m_vs);
        
        av::Codec codec = av::findDecodingCodec(m_vdec.raw()->codec_id);
        m_vdec.setCodec(codec);
        m_vdec.setRefCountedFrames(true);
        
        m_vdec.open();
      }
    }
    
    av::VideoFrame next() {
      av::Packet pk;
      av::VideoFrame res;
      // read packets until a video packet is found
      fmt::print("Video stream: {}\n", m_vs_idx);
      do {
        bool is_eof = true;
        while ((pk = m_fmt.readPacket())) {
          fmt::print("Reading packet from stream {}\n", pk.streamIndex());
          if (pk.streamIndex() == m_vs_idx) {
            is_eof = false;
            break;
          }
        }
        if (is_eof) {
          res = av::VideoFrame(nullptr);
          break;
        }
        res = m_vdec.decode(pk);
      } while (res.isNull() || !res.isComplete());
      fmt::print("Frame size: {}x{}\n", res.width(), res.height());
      return res;
    }
    
    av::FormatContext m_fmt;
    av::VideoDecoderContext m_vdec;
    av::Stream m_vs;
    size_t m_vs_idx;
  };
  
  video_decoder::video_decoder(const fs::path& path) :
    p_impl(new impl(path)) {}
    
  video_decoder::~video_decoder() {
    delete p_impl;
  }
  
  av::VideoFrame video_decoder::next() {
    return p_impl->next();
  }
}