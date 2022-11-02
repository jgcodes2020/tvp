#ifndef TVP_DITHER_HPP_INCLUDED
#define TVP_DITHER_HPP_INCLUDED

#define CL_HPP_TARGET_OPENCL_VERSION 210
#define CL_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>
#include <CL/opencl.hpp>

#include "ffmpeg.hpp"

namespace tvp {
  void init_palette(AVFrame* dst);
  void dither(const AVFrame* src, AVFrame* dst);
}
#endif