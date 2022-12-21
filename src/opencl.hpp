#ifndef TVP_OPENCL_HPP_INCLUDED
#define TVP_OPENCL_HPP_INCLUDED

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
  
  struct ocl_globals {
    ocl_globals();
    
    inline static ocl_globals& get() {
      static ocl_globals g;
      return g;
    }
    
    cl::Platform plat;
    cl::Device dev;
    cl::Program prog;
  };
  
  class dither_context {
  public:
    dither_context() : 
      m_ctx(_s().dev), 
      m_cq(m_ctx),
      m_knl(_s().prog, "ordered_dither")
      {}
      
    void dither(const AVFrame* src, AVFrame* dst);
    
  private:
    inline static ocl_globals& _s() {
      return ocl_globals::get();
    }
    
    cl::Context m_ctx;
    cl::CommandQueue m_cq;
    cl::Kernel m_knl;
  };
}
#endif