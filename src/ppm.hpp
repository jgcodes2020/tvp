#ifndef TVP_PPM_HPP
#define TVP_PPM_HPP
#include "frame.h"
#include <filesystem>

namespace tvp {
  void encode_ppm(const av::VideoFrame& frame, const std::filesystem::path& path);
}
#endif