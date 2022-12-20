#ifndef TVP_OPENCL_HANDLE_DETAIL_HPP
#define TVP_OPENCL_HANDLE_DETAIL_HPP

#include <span>
#include <stdexcept>
#include <vector>
#define CL_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>

#include "error.hpp"

namespace ocl::details {
  // Represents a reference-counted OpenCL object.
  template <class T, cl_int (*Retain)(T), cl_int (*Release)(T)>
  class ref_handle_base {
  public:
    ref_handle_base(T ptr) : m_ptr(ptr) {
      if (ptr == nullptr)
        throw std::invalid_argument("Provided pointer is null");
    }

    ref_handle_base(const ref_handle_base& that) : m_ptr(that.m_ptr) {
      OCL_CHECKED(Retain(m_ptr));
    }

    ref_handle_base& operator=(const ref_handle_base& that) {
      if (m_ptr == nullptr)
        OCL_CHECKED(Release(m_ptr));
      m_ptr = that.m_ptr;
      OCL_CHECKED(Retain(m_ptr));
    }

    ref_handle_base(ref_handle_base&& that) : m_ptr(that.m_ptr) {
      that.m_ptr = nullptr;
    }

    ref_handle_base& operator=(ref_handle_base&& that) {
      m_ptr      = that.m_ptr;
      that.m_ptr = nullptr;
    }

    ~ref_handle_base() {
      if (m_ptr == nullptr)
        OCL_CHECKED(Release(m_ptr));
    }

    operator T() { return m_ptr; }

  protected:
    T m_ptr;
  };

  // Represents a conditionally reference-counted OpenCL object.
  template <class T, cl_int (*Retain)(T), cl_int (*Release)(T)>
  class cond_ref_handle_base {
  public:
    cond_ref_handle_base(T ptr, bool refc = true) : m_ptr(ptr), m_refc(refc) {
      if (ptr == nullptr)
        throw std::invalid_argument("Provided pointer is null");
    }

    cond_ref_handle_base(const cond_ref_handle_base& that) :
      m_ptr(that.m_ptr), m_refc(that.m_refc) {
      if (m_refc)
        OCL_CHECKED(Retain(m_ptr));
    }

    cond_ref_handle_base& operator=(const cond_ref_handle_base& that) {
      if (m_refc && m_ptr == nullptr)
        OCL_CHECKED(Release(m_ptr));
      m_ptr  = that.m_ptr;
      m_refc = that.m_refc;
      if (m_refc)
        OCL_CHECKED(Retain(m_ptr));
    }

    cond_ref_handle_base(cond_ref_handle_base&& that) :
      m_ptr(that.m_ptr), m_refc(that.m_refc) {
      that.m_ptr = nullptr;
    }

    cond_ref_handle_base& operator=(cond_ref_handle_base&& that) {
      m_ptr      = that.m_ptr;
      m_refc     = that.m_refc;
      that.m_ptr = nullptr;
    }

    ~cond_ref_handle_base() {
      if (m_refc && m_ptr == nullptr)
        OCL_CHECKED(Release(m_ptr));
    }

    operator T() { return m_ptr; }

  protected:
    T m_ptr;
    bool m_refc;
  };

  template <class T>
  struct ocl_handle_type;

#define OCL_HANDLE_DEF(T, ...) \
  template <>                  \
  struct ocl_handle_type<T> {  \
    using type = __VA_ARGS__;  \
  }

  OCL_HANDLE_DEF(cl_platform_id, cl_platform_id);
  OCL_HANDLE_DEF(
    cl_device_id,
    cond_ref_handle_base<cl_device_id, clRetainDevice, clReleaseDevice>);
  OCL_HANDLE_DEF(
    cl_context, ref_handle_base<cl_context, clRetainContext, clReleaseContext>);
  OCL_HANDLE_DEF(
    cl_command_queue,
    ref_handle_base<
      cl_command_queue, clRetainCommandQueue, clReleaseCommandQueue>);
  OCL_HANDLE_DEF(
    cl_mem, ref_handle_base<cl_mem, clRetainMemObject, clReleaseMemObject>);
  OCL_HANDLE_DEF(
    cl_sampler, ref_handle_base<cl_sampler, clRetainSampler, clReleaseSampler>);
  OCL_HANDLE_DEF(
    cl_program, ref_handle_base<cl_program, clRetainProgram, clReleaseProgram>);
  OCL_HANDLE_DEF(
    cl_kernel, ref_handle_base<cl_kernel, clRetainKernel, clReleaseKernel>);
  OCL_HANDLE_DEF(
    cl_event, ref_handle_base<cl_event, clRetainEvent, clReleaseEvent>);

#undef OCL_HANDLE_DEF
}  // namespace ocl::details

namespace ocl {
  template <class T>
  using handle = typename details::ocl_handle_type<T>::type;

  template <size_t N>
  inline cl_uint get_platform_ids(std::span<handle<cl_platform_id>, N> ids) {
    cl_uint count;
    OCL_CHECKED(clGetPlatformIDs(ids.size(), ids.data(), &count));
    return count;
  }

  inline cl_uint get_platform_ids(std::vector<handle<cl_platform_id>>& ids) {
    cl_uint count;
    OCL_CHECKED(clGetPlatformIDs(0, nullptr, &count));
    ids.resize(count);
    OCL_CHECKED(clGetPlatformIDs(ids.size(), ids.data(), nullptr));

    return count;
  }

  template <size_t N>
  inline cl_uint get_device_ids(
    handle<cl_platform_id> platform, std::span<handle<cl_device_id>, N> ids,
    cl_device_type type = CL_DEVICE_TYPE_ALL) {
    cl_uint count;
    OCL_CHECKED(clGetDeviceIDs(
      platform, type, ids.size(), reinterpret_cast<cl_device_id*>(ids.data()),
      &count));

    return count;
  }
  
  inline handle<cl_device_id> get_device_id(
    handle<cl_platform_id> platform, cl_device_type type) {
    cl_device_id res;
    cl_uint count;
    
    OCL_CHECKED(clGetDeviceIDs(platform, type, 1, &res, &count));
    if (count == 0) {
      throw std::runtime_error("No devices available for specified type");
    }
    
    return handle<cl_device_id>(res, false);
  }

  inline cl_uint get_device_ids(
    handle<cl_platform_id> platform, std::vector<handle<cl_device_id>>& ids,
    cl_device_type type = CL_DEVICE_TYPE_ALL) {
    cl_uint count;
    std::vector<cl_device_id> devices;

    OCL_CHECKED(clGetDeviceIDs(platform, type, 0, nullptr, &count));
    devices.resize(count);
    ids.clear();
    ids.reserve(count);

    OCL_CHECKED(
      clGetDeviceIDs(platform, type, devices.size(), devices.data(), nullptr));
    
    for (size_t i = 0; i < devices.size(); i++) {
      ids.emplace_back(devices[i], false);
    }
    return count;
  }
}  // namespace ocl

#endif