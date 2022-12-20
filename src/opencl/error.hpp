#ifndef TVP_OPENCL_ERROR_HPP
#define TVP_OPENCL_ERROR_HPP

#include <system_error>
#include <cstdint>

#define CL_TARGET_OPENCL_VERSION 210
#include <CL/cl.h>

#define OCL_CHECK(cond) if (cl_int err = (cond); err != CL_SUCCESS)
#define OCL_CHECKED(cond) \
  OCL_CHECK(cond) throw ::std::system_error(err, ::ocl::ocl_category())
#define OCL_CHECKED_THEN(cond) OCL_CHECKED(cond); else

namespace ocl {
  enum class ocl_errc : cl_int {
    success = 0,
    device_not_found = -1,
    device_not_available = -2,
    compiler_not_available = -3,
    mem_object_allocation_failure = -4,
    out_of_resources = -5,
    out_of_host_memory = -6,
    profiling_info_not_available = -7,
    mem_copy_overlap = -8,
    image_format_mismatch = -9,
    image_format_not_supported = -10,
    build_program_failure = -11,
    map_failure = -12,
    misaligned_sub_buffer_offset = -13,
    exec_status_error_for_events_in_wait_list = -14,
    compile_program_failure = -15,
    linker_not_available = -16,
    link_program_failure = -17,
    device_partition_failed = -18,
    kernel_arg_info_not_available = -19,

    invalid_value = -30,
    invalid_device_type = -31,
    invalid_platform = -32,
    invalid_device = -33,
    invalid_context = -34,
    invalid_queue_properties = -35,
    invalid_command_queue = -36,
    invalid_host_ptr = -37,
    invalid_mem_object = -38,
    invalid_image_format_descriptor = -39,
    invalid_image_size = -40,
    invalid_sampler = -41,
    invalid_binary = -42,
    invalid_build_options = -43,
    invalid_program = -44,
    invalid_program_executable = -45,
    invalid_kernel_name = -46,
    invalid_kernel_definition = -47,
    invalid_kernel = -48,
    invalid_arg_index = -49,
    invalid_arg_value = -50,
    invalid_arg_size = -51,
    invalid_kernel_args = -52,
    invalid_work_dimension = -53,
    invalid_work_group_size = -54,
    invalid_work_item_size = -55,
    invalid_global_offset = -56,
    invalid_event_wait_list = -57,
    invalid_event = -58,
    invalid_operation = -59,
    invalid_gl_object = -60,
    invalid_buffer_size = -61,
    invalid_mip_level = -62,
    invalid_global_work_size = -63,

    invalid_property = -64,

    invalid_image_descriptor = -65,
    invalid_compiler_options = -66,
    invalid_linker_options = -67,
    invalid_device_partition_count = -68,

    invalid_pipe_size = -69,
    invalid_device_queue = -70,

    invalid_spec_id = -71,
    max_size_restriction_exceeded = -72
  };
  
  
  namespace details {
    class ocl_category : public std::error_category {
    public:
      using std::error_category::error_category;
      virtual ~ocl_category() override {}
    
      const char* name() const noexcept override {
        return "OpenCL";
      }
      
      std::string message(int cond) const noexcept override{
        switch (static_cast<ocl_errc>(cond)) {
        case ocl_errc::success:
          return "Success";
        case ocl_errc::device_not_found:
          return "No device found";
        case ocl_errc::device_not_available:
          return "Device not available";
        case ocl_errc::compiler_not_available:
          return "Compiler not available";
        case ocl_errc::mem_object_allocation_failure:
          return "Memory allocation on target device failed";
        case ocl_errc::out_of_resources:
          return "Not enough resources";
        case ocl_errc::out_of_host_memory:
          return "Memory allocation on host failed";
        case ocl_errc::profiling_info_not_available:
          return "Profiling info not available";
        case ocl_errc::mem_copy_overlap:
          return "Buffer copy pointers overlap";
        case ocl_errc::image_format_mismatch:
          return "Incorrect image format";
        case ocl_errc::image_format_not_supported:
          return "Image format is not supported";
        case ocl_errc::build_program_failure:
          return "Compute program build failed";
        case ocl_errc::map_failure:
          return "Map failed";
        case ocl_errc::misaligned_sub_buffer_offset:
          return "Misaligned sub-buffer offset";
        case ocl_errc::exec_status_error_for_events_in_wait_list:
          return "Error for events in waitlist";
        case ocl_errc::compile_program_failure:
          return "Compute program compile failed";
        case ocl_errc::linker_not_available:
          return "Linker not available";
        case ocl_errc::link_program_failure:
          return "Compute program link failed";
        case ocl_errc::device_partition_failed:
          return "Device partitioning failed";
        case ocl_errc::kernel_arg_info_not_available:
          return "Kernel argument info not available";
        case ocl_errc::invalid_value:
          return "Invalid argument";
        case ocl_errc::invalid_device_type:
          return "Invalid CL_DEVICE_TYPE_* value";
        case ocl_errc::invalid_platform:
          return "Invalid cl_platform_id";
        case ocl_errc::invalid_device:
          return "Invalid cl_device_id";
        case ocl_errc::invalid_context:
          return "Invalid cl_context";
        case ocl_errc::invalid_queue_properties:
          return "Invalid cl_queue_properties";
        case ocl_errc::invalid_command_queue:
          return "Invalid cl_command_queue";
        case ocl_errc::invalid_host_ptr:
          return "Invalid pointer to host memory";
        case ocl_errc::invalid_mem_object:
          return "Invalid device memory object";
        case ocl_errc::invalid_image_format_descriptor:
          return "Invalid image description";
        case ocl_errc::invalid_image_size:
          return "Invalid image size";
        case ocl_errc::invalid_sampler:
          return "Invalid image sampler";
        case ocl_errc::invalid_binary:
          return "Invalid device binary";
        case ocl_errc::invalid_build_options:
          return "Invalid program build options";
        case ocl_errc::invalid_program:
          return "Invalid cl_program";
        case ocl_errc::invalid_program_executable:
          return "Invalid program executable";
        case ocl_errc::invalid_kernel_name:
          return "Kernel name doesn't exist";
        case ocl_errc::invalid_kernel_definition:
          return "Invalid kernel definition";
        case ocl_errc::invalid_kernel:
          return "Invalid cl_kernel";
        case ocl_errc::invalid_arg_index:
          return "Kernel argument index out-of-bounds";
        case ocl_errc::invalid_arg_value:
          return "Invalid kernel argument value";
        case ocl_errc::invalid_arg_size:
          return "Invalid argument size";
        case ocl_errc::invalid_kernel_args:
          return "Invalid kernel arguments";
        case ocl_errc::invalid_work_dimension:
          return "Invalid work dimension";
        case ocl_errc::invalid_work_group_size:
          return "Invalid workgroup size";
        case ocl_errc::invalid_work_item_size:
          return "Invalid work item size";
        case ocl_errc::invalid_global_offset:
          return "Invalid global offset";
        case ocl_errc::invalid_event_wait_list:
          return "Invalid event waitlist";
        case ocl_errc::invalid_event:
          return "Invalid event";
        case ocl_errc::invalid_operation:
          return "Invalid operation";
        case ocl_errc::invalid_gl_object:
          return "Invalid OpenGL object";
        case ocl_errc::invalid_buffer_size:
          return "Invalid buffer size";
        case ocl_errc::invalid_mip_level:
          return "Invalid mipmapping level";
        case ocl_errc::invalid_global_work_size:
          return "Invalid global work size";

        case ocl_errc::invalid_property:
          return "Invalid property";

        case ocl_errc::invalid_image_descriptor:
          return "Invalid image descriptor";
        case ocl_errc::invalid_compiler_options:
          return "Invalid OpenCL compiler options";
        case ocl_errc::invalid_linker_options:
          return "Invalid OpenCL linker options";
        case ocl_errc::invalid_device_partition_count:
          return "Invalid device partition count";

        case ocl_errc::invalid_pipe_size:
          return "Invalid pipe size";
        case ocl_errc::invalid_device_queue:
          return "Invalid device queue size";

        case ocl_errc::invalid_spec_id:
          return "Invalid spec ID";
        case ocl_errc::max_size_restriction_exceeded:
          return "Max size restriction exceeded";
        }
      }
    };
  }
  
  inline std::error_category& ocl_category() {
    static details::ocl_category category;
    return category;
  }
}

#endif