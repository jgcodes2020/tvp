#include "ppm.hpp"
#include "averror.h"
#include "videorescaler.h"

#include <fmt/core.h>
#include <libavutil/pixdesc.h>
#include <libavutil/pixfmt.h>

#include <fstream>
#include <iterator>

namespace tvp {
  void encode_ppm(const av::VideoFrame& in_frame, const std::filesystem::path& path) {
    std::ofstream ofs(path, std::ios_base::out | std::ios_base::binary);
    auto oi = std::ostreambuf_iterator(ofs);
    
    av::VideoFrame frame;
    if (in_frame.pixelFormat() == AV_PIX_FMT_RGB24) {
      frame = in_frame;
    }
    else {
      fmt::print("Pixel format: {}\n", av_get_pix_fmt_name(in_frame.pixelFormat()));
      av::VideoRescaler scaler(
        in_frame.width(), in_frame.height(), AV_PIX_FMT_RGB24);
      frame = scaler.rescale(in_frame, av::throws());
    }
    
    fmt::format_to(oi, "P6 {} {} 255\n", frame.width(), frame.height());
    ofs.write((char*) frame.data(0), frame.size(0));
  }
}