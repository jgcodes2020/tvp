#ifndef VKC_CONTEXT_HPP
#define VKC_CONTEXT_HPP

#include <cstring>
#include <filesystem>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace vkc {
  class instance;
  extern const char default_app_name[];

  struct vk_unmapper {
    void operator()(void* p) { dev->unmapMemory(*dmem); }
    const vk::Device* dev;
    const vk::DeviceMemory* dmem;
  };

  template <class T>
  using dmem_ptr = std::unique_ptr<T, vk_unmapper>;

  class buffer {
  public:
    static buffer alloc(const std::shared_ptr<instance>& inst, size_t size);

    // copy data from an external buffer
    void copy_in(uint32_t dst_off, void* src, uint32_t size) {
      auto ptr = this->map();
      std::memcpy(((char*) ptr.get()) + dst_off, src, size);
    }
    // copy data to an external buffer
    void copy_out(void* dst, uint32_t src_off, uint32_t size) {
      auto ptr = this->map();
      std::memcpy(dst, ((char*) ptr.get()) + src_off, size);
    }

    template <class T = void>
    dmem_ptr<T> map();

  private:
    buffer(
      const std::shared_ptr<instance>& inst, vk::Buffer&& buf,
      vk::DeviceSize size, vk::DeviceMemory&& dmem) :
      m_inst(inst), m_buf(buf), m_dsize(), m_dmem(dmem) {}

    std::shared_ptr<instance> m_inst;
    vk::Buffer m_buf;
    vk::DeviceSize m_dsize;
    vk::DeviceMemory m_dmem;
  };

  class shader {
  public:
    static shader load(const std::filesystem::path& path);
  private:
    shader(
      const std::shared_ptr<instance>& inst, vk::ShaderModule&& sdm,
      vk::DescriptorSetLayout&& dsl, vk::PipelineLayout&& plnl,
      vk::Pipeline&& pln) :
      m_inst(inst), m_sdm(sdm), m_dsl(dsl), m_plnl(plnl), m_pln(pln) {}

    std::shared_ptr<instance> m_inst;
    vk::ShaderModule m_sdm;
    vk::DescriptorSetLayout m_dsl;
    vk::PipelineLayout m_plnl;
    vk::Pipeline m_pln;
  };

  class instance {
    friend class buffer;

  public:
    ~instance() {
      m_dev.destroy();
      m_inst.destroy();
    }

    static instance init(
      uint32_t vk_version = VK_API_VERSION_1_1,
      const char* name = default_app_name, uint32_t version = 1);

  private:
    // Manually init a vkc::instance.
    instance(
      vk::Instance&& inst, vk::PhysicalDevice&& pdev, uint32_t cqfi,
      vk::Device&& dev, uint32_t mti, vk::DeviceSize mhs) :
      m_inst(inst), m_pdev(pdev), m_cqfi(cqfi), m_dev(dev), m_mti() {}

    vk::Instance m_inst;
    vk::PhysicalDevice m_pdev;
    // compute queue family index
    uint32_t m_cqfi;
    vk::Device m_dev;
    // memory type index
    uint32_t m_mti;
    // memory heap size
    vk::DeviceSize m_mhs;
  };

  // FUNCTION DEFINITIONS
  // ====================

  template <class T>
  dmem_ptr<T> buffer::map() {
    void* ptr = m_inst->m_dev.mapMemory(m_dmem, 0, m_dsize);
    return dmem_ptr<T>(ptr, vk_unmapper {&m_inst->m_dev, &m_dmem});
  }
}  // namespace vkc

#endif