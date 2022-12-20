#ifndef TVP_DITHER_HPP_INCLUDED
#define TVP_DITHER_HPP_INCLUDED

#include <libavutil/imgutils.h>
#include <stdexcept>
#include <vector>
#define CL_HPP_TARGET_OPENCL_VERSION 210
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>
#include <CL/opencl.hpp>

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(tvp);

#include "ffmpeg.hpp"

namespace tvp {
  void init_palette(AVFrame* dst);
  
  class dither_context {
  public:
    dither_context() : 
      m_ctx(_s().dev), 
      m_cq(m_ctx),
      m_knl(_s().prog, "ordered_dither")
      {}
      
    void dither(const AVFrame* src, AVFrame* dst);
    
  private:
    struct stat_info {
    public:
      stat_info() : 
        plat(cl::Platform::getDefault()),
        dev([this]() {
          std::vector<cl::Device> devs {};
          plat.getDevices(CL_DEVICE_TYPE_GPU, &devs);
          return devs[0];
        }()),
        prog([this]() {
          cl::Program::Sources srcs;
          auto fs = cmrc::tvp::get_filesystem();
          auto file = fs.open("dither.ocl");
          srcs.push_back({file.begin(), file.size()});
          return cl::Program(srcs);
        }()) {
        prog.build();
      }
      cl::Platform plat;
      cl::Device dev;
      cl::Program prog;
    };
    static stat_info& _s() {
      static stat_info s;
      return s;
    }
    
    cl::Context m_ctx;
    cl::CommandQueue m_cq;
    cl::Kernel m_knl;
  };
}
#endif