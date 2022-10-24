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
  public:
    video_decoder(std::filesystem::path path);
    
    av::VideoFrame next();
    
  private:
    struct impl;
    std::unique_ptr<impl> p_impl;
  };
}
#endif