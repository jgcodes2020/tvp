#ifndef TVP_PPM_HPP_INCLUDED
#define TVP_PPM_HPP_INCLUDED
#include <fmt/core.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include "ffmpeg.hpp"
namespace tvp {
  inline void save_ppm(const std::filesystem::path& path, const AVFrame* frame) {
    if (frame->format != AV_PIX_FMT_RGB24)
      throw std::invalid_argument("frame format != RGB24");
    
    std::ofstream out(path, std::ios_base::out | std::ios_base::binary);
    
    fmt::format_to(std::ostreambuf_iterator(out), "P6 {} {} 255\n", frame->width, frame->height);
    out.write((char*) frame->buf[0]->data, frame->buf[0]->size);
    out.flush();
  }
}
#endif