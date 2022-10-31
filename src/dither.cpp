#include "dither.hpp"
#include <optional>
#include <stdexcept>
#include <vector>

#define CL_HPP_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>
#include <CL/opencl.hpp>

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(tvp);

namespace {
  decltype(auto) cl_globals() {
    struct out_value {
      cl::Device device;
      cl::Context context;
    };

    static std::optional<out_value> res {};
    [[unlikely]] if (!res) {
      auto plat = cl::Platform::getDefault();

      std::vector<cl::Device> devices;
      plat.getDevices(CL_DEVICE_TYPE_ALL, &devices);

      res = out_value {devices[0], cl::Context(devices[0])};
    }

    return (const out_value&) *res;
  }

  cl::Program dither_kernel() {
    static std::optional<cl::Program> res;
    [[unlikely]] if (!res) {
      auto fs   = cmrc::tvp::get_filesystem();
      auto file = fs.open("dither_kernel.ocl");

      // Arrays for C API call
      const char* src_ptrs[1] {file.begin()};
      size_t src_sizes[1] {file.size()};
      
      // C API call
      cl_program c_obj = clCreateProgramWithSource(
        cl_globals().context.get(), 1, src_ptrs, src_sizes, nullptr);
      if (c_obj == nullptr) {
        throw std::runtime_error("OpenCL error");
      }
      res.emplace(c_obj);
    }
    return *res;
  }
}  // namespace