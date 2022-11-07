#include "dither.hpp"
#include <fmt/core.h>
#include <optional>
#include <stdexcept>
#include <vector>

#define CL_HPP_TARGET_OPENCL_VERSION 210
#define CL_HPP_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>
#include <CL/opencl.hpp>

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(tvp);

namespace {
  decltype(auto) cl_globals() {
    struct out_value {
      cl::Device device;
      cl::Context context;
      cl::CommandQueue queue;
    };

    static std::optional<out_value> res {};
    [[unlikely]] if (!res) {
      auto plat = cl::Platform::getDefault();

      std::vector<cl::Device> devices;
      plat.getDevices(CL_DEVICE_TYPE_ALL, &devices);
      if (devices.size() == 0)
        throw std::runtime_error("No suitable OpenCL devices found");

      res =
        out_value {.device = devices[0], .context = cl::Context(devices[0])};
      res->queue = cl::CommandQueue(res->context, res->device);
    }

    return (out_value&) *res;
  }

  cl::Context& get_context() {
    return cl_globals().context;
  }

  cl::CommandQueue& get_queue() {
    return cl_globals().queue;
  }

  cl::Program dither_kernel() {
    static std::optional<cl::Program> res;
    [[unlikely]] if (!res) {
      int rc = 0;
      
      auto fs   = cmrc::tvp::get_filesystem();
      auto file = fs.open("dither_kernel.ocl");

      // Arrays for C API call
      const char* src_ptrs[1] {file.begin()};
      size_t src_sizes[1] {file.size()};

      // C API call
      cl_program c_obj = clCreateProgramWithSource(
        cl_globals().context.get(), 1, src_ptrs, src_sizes, &rc);
      if (rc != CL_SUCCESS) {
        throw std::runtime_error(fmt::format("OpenCL error (code {})", rc));
      }
      res.emplace(c_obj);
      
      try {
        res->build();
      }
      catch (const cl::BuildError& err) {
        fmt::print("OpenCL build error:\n{}\n", err.getBuildLog().at(0).second);
        std::abort();
      }
    }
    return *res;
  }

  // Palette data in packed 0xAARRGGBB
  const std::array<uint32_t, 256> xterm_palette = {
    0xFF000000, 0xFF800000, 0xFF008000, 0xFF808000, 0xFF000080, 0xFF800080,
    0xFF008080, 0xFFC0C0C0, 0xFF808080, 0xFFFF0000, 0xFF00FF00, 0xFFFFFF00,
    0xFF0000FF, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF, 0xFF000000, 0xFF00005F,
    0xFF000087, 0xFF0000AF, 0xFF0000D7, 0xFF0000FF, 0xFF005F00, 0xFF005F5F,
    0xFF005F87, 0xFF005FAF, 0xFF005FD7, 0xFF005FFF, 0xFF008700, 0xFF00875F,
    0xFF008787, 0xFF0087AF, 0xFF0087D7, 0xFF0087FF, 0xFF00AF00, 0xFF00AF5F,
    0xFF00AF87, 0xFF00AFAF, 0xFF00AFD7, 0xFF00AFFF, 0xFF00D700, 0xFF00D75F,
    0xFF00D787, 0xFF00D7AF, 0xFF00D7D7, 0xFF00D7FF, 0xFF00FF00, 0xFF00FF5F,
    0xFF00FF87, 0xFF00FFAF, 0xFF00FFD7, 0xFF00FFFF, 0xFF5F0000, 0xFF5F005F,
    0xFF5F0087, 0xFF5F00AF, 0xFF5F00D7, 0xFF5F00FF, 0xFF5F5F00, 0xFF5F5F5F,
    0xFF5F5F87, 0xFF5F5FAF, 0xFF5F5FD7, 0xFF5F5FFF, 0xFF5F8700, 0xFF5F875F,
    0xFF5F8787, 0xFF5F87AF, 0xFF5F87D7, 0xFF5F87FF, 0xFF5FAF00, 0xFF5FAF5F,
    0xFF5FAF87, 0xFF5FAFAF, 0xFF5FAFD7, 0xFF5FAFFF, 0xFF5FD700, 0xFF5FD75F,
    0xFF5FD787, 0xFF5FD7AF, 0xFF5FD7D7, 0xFF5FD7FF, 0xFF5FFF00, 0xFF5FFF5F,
    0xFF5FFF87, 0xFF5FFFAF, 0xFF5FFFD7, 0xFF5FFFFF, 0xFF870000, 0xFF87005F,
    0xFF870087, 0xFF8700AF, 0xFF8700D7, 0xFF8700FF, 0xFF875F00, 0xFF875F5F,
    0xFF875F87, 0xFF875FAF, 0xFF875FD7, 0xFF875FFF, 0xFF878700, 0xFF87875F,
    0xFF878787, 0xFF8787AF, 0xFF8787D7, 0xFF8787FF, 0xFF87AF00, 0xFF87AF5F,
    0xFF87AF87, 0xFF87AFAF, 0xFF87AFD7, 0xFF87AFFF, 0xFF87D700, 0xFF87D75F,
    0xFF87D787, 0xFF87D7AF, 0xFF87D7D7, 0xFF87D7FF, 0xFF87FF00, 0xFF87FF5F,
    0xFF87FF87, 0xFF87FFAF, 0xFF87FFD7, 0xFF87FFFF, 0xFFAF0000, 0xFFAF005F,
    0xFFAF0087, 0xFFAF00AF, 0xFFAF00D7, 0xFFAF00FF, 0xFFAF5F00, 0xFFAF5F5F,
    0xFFAF5F87, 0xFFAF5FAF, 0xFFAF5FD7, 0xFFAF5FFF, 0xFFAF8700, 0xFFAF875F,
    0xFFAF8787, 0xFFAF87AF, 0xFFAF87D7, 0xFFAF87FF, 0xFFAFAF00, 0xFFAFAF5F,
    0xFFAFAF87, 0xFFAFAFAF, 0xFFAFAFD7, 0xFFAFAFFF, 0xFFAFD700, 0xFFAFD75F,
    0xFFAFD787, 0xFFAFD7AF, 0xFFAFD7D7, 0xFFAFD7FF, 0xFFAFFF00, 0xFFAFFF5F,
    0xFFAFFF87, 0xFFAFFFAF, 0xFFAFFFD7, 0xFFAFFFFF, 0xFFD70000, 0xFFD7005F,
    0xFFD70087, 0xFFD700AF, 0xFFD700D7, 0xFFD700FF, 0xFFD75F00, 0xFFD75F5F,
    0xFFD75F87, 0xFFD75FAF, 0xFFD75FD7, 0xFFD75FFF, 0xFFD78700, 0xFFD7875F,
    0xFFD78787, 0xFFD787AF, 0xFFD787D7, 0xFFD787FF, 0xFFD7AF00, 0xFFD7AF5F,
    0xFFD7AF87, 0xFFD7AFAF, 0xFFD7AFD7, 0xFFD7AFFF, 0xFFD7D700, 0xFFD7D75F,
    0xFFD7D787, 0xFFD7D7AF, 0xFFD7D7D7, 0xFFD7D7FF, 0xFFD7FF00, 0xFFD7FF5F,
    0xFFD7FF87, 0xFFD7FFAF, 0xFFD7FFD7, 0xFFD7FFFF, 0xFFFF0000, 0xFFFF005F,
    0xFFFF0087, 0xFFFF00AF, 0xFFFF00D7, 0xFFFF00FF, 0xFFFF5F00, 0xFFFF5F5F,
    0xFFFF5F87, 0xFFFF5FAF, 0xFFFF5FD7, 0xFFFF5FFF, 0xFFFF8700, 0xFFFF875F,
    0xFFFF8787, 0xFFFF87AF, 0xFFFF87D7, 0xFFFF87FF, 0xFFFFAF00, 0xFFFFAF5F,
    0xFFFFAF87, 0xFFFFAFAF, 0xFFFFAFD7, 0xFFFFAFFF, 0xFFFFD700, 0xFFFFD75F,
    0xFFFFD787, 0xFFFFD7AF, 0xFFFFD7D7, 0xFFFFD7FF, 0xFFFFFF00, 0xFFFFFF5F,
    0xFFFFFF87, 0xFFFFFFAF, 0xFFFFFFD7, 0xFFFFFFFF, 0xFF080808, 0xFF121212,
    0xFF1C1C1C, 0xFF262626, 0xFF303030, 0xFF3A3A3A, 0xFF444444, 0xFF4E4E4E,
    0xFF585858, 0xFF626262, 0xFF6C6C6C, 0xFF767676, 0xFF808080, 0xFF8A8A8A,
    0xFF949494, 0xFF9E9E9E, 0xFFA8A8A8, 0xFFB2B2B2, 0xFFBCBCBC, 0xFFC6C6C6,
    0xFFD0D0D0, 0xFFDADADA, 0xFFE4E4E4, 0xFFEEEEEE,
  };
}  // namespace

namespace tvp {
  void init_palette(AVFrame* dst) {
    if (dst->format != AV_PIX_FMT_PAL8)
      throw std::invalid_argument("dst format != PAL8");
    memcpy(dst->buf[1]->data, xterm_palette.data(), sizeof(xterm_palette));
  }

  void dither(const AVFrame* src, AVFrame* dst) {
    if (src->format != AV_PIX_FMT_RGB0)
      throw std::invalid_argument("src format != RGB0");
    if (dst->format != AV_PIX_FMT_PAL8)
      throw std::invalid_argument("dst format != PAL8");

    cl::Kernel kernel {dither_kernel(), "ordered_dither"};
    cl::Buffer src_buf(get_context(), CL_MEM_READ_ONLY, src->linesize[0] * src->height);
    cl::Buffer dst_buf(get_context(), CL_MEM_READ_WRITE, dst->linesize[0] * dst->height);
    
    get_queue().enqueueWriteBuffer(
      src_buf, CL_TRUE, 0, src->linesize[0] * src->height, src->data[0]);

    kernel.setArg(0, src_buf);
    kernel.setArg(1, dst_buf);

    get_queue().enqueueNDRangeKernel(
      kernel, cl::NullRange, cl::NDRange(src->width, src->height));
    get_queue().finish();
    
    get_queue().enqueueReadBuffer(
      dst_buf, CL_TRUE, 0, dst->linesize[0] * dst->height, dst->data[0]);
  }
}  // namespace tvp