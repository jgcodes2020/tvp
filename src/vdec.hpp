#ifndef TVP_VDEC_HPP
#define TVP_VDEC_HPP

#include <filesystem>
#include <memory>
#include <optional>

#include <av.h>
#include <packet.h>
#include <format.h>
#include <formatcontext.h>
#include <codec.h>
#include <codeccontext.h>

namespace tvp {
  class video_decoder {
  private:
    struct impl;
    impl* p_impl;
    
  public:
    video_decoder(const std::filesystem::path& path);
    
    ~video_decoder();
    
    av::VideoFrame next();
  };
}
#endif