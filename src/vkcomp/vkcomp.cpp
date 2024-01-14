#include "vkcomp.hpp"
#include <vulkan/vulkan_core.h>
#include <fstream>
#include <ios>
#include <limits>
#include <optional>
#include <utility>
#include <vector>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace {
  inline vk::Instance init_instance(
    uint32_t vk_version, const char* app_name, uint32_t app_version) {
    vk::ApplicationInfo app_info {
      app_name, app_version, nullptr, 0, vk_version};
    std::vector<const char*> layers {"VK_LAYER_KHRONOS_validation"};

    vk::InstanceCreateInfo inst_info(
      vk::InstanceCreateFlags(), &app_info, layers, {});
    return vk::createInstance(inst_info);
  }

  // True if this Vulkan device can do compute.
  inline std::optional<size_t> pdev_find_compute(
    const vk::PhysicalDevice& dev) {
    const auto& queue_props = dev.getQueueFamilyProperties();
    for (size_t i = 0; i < queue_props.size(); i++) {
      if (queue_props[i].queueFlags & vk::QueueFlagBits::eCompute)
        return i;
    }
    return std::nullopt;
  }

  inline int pdev_priority(const vk::PhysicalDevice& dev) {
    static const int priorities[] = {
      20,  // other
      80,  // integrated GPU
      90,  // discrete GPU
      70,  // virtual GPU (e.g. via SR-IOV)
      0,   // CPU
    };
    int idx = (int) dev.getProperties().deviceType;

    return (idx < 0 || idx > ((sizeof(priorities) / sizeof(int))))
      ? -100
      : priorities[idx];
  }

  inline decltype(auto) get_device_config(const vk::Instance& inst) {
    const vk::PhysicalDevice* best = nullptr;
    int best_priority = std::numeric_limits<int>::min();
    size_t best_queue_config;
    for (const auto& pdev : inst.enumeratePhysicalDevices()) {
      auto comp_check = pdev_find_compute(pdev);
      if (!comp_check)
        continue;

      if (best == nullptr) {
        best = &pdev;
        best_priority = pdev_priority(pdev);
        best_queue_config = *comp_check;
      }
      else if (pdev_priority(pdev) > best_priority) {
        best = &pdev;
        best_priority = pdev_priority(pdev);
        best_queue_config = *comp_check;
      }
    }

    struct {
      vk::PhysicalDevice dev;
      size_t config;
    } rval {*best, best_queue_config};
    return rval;
  }

  inline vk::Device make_device(
    const decltype(get_device_config(std::declval<vk::Instance>()))& obj) {
    vk::DeviceQueueCreateInfo dqci({}, obj.config, 1);
    vk::DeviceCreateInfo dci({}, dqci);

    return obj.dev.createDevice(dci);
  }

  inline decltype(auto) get_mem_info(const vk::PhysicalDevice& pdev) {
    struct {
      uint32_t mti;
      vk::DeviceSize mhs;
    } res;

    constexpr vk::MemoryPropertyFlags mem_flags =
      vk::MemoryPropertyFlagBits::eHostCoherent |
      vk::MemoryPropertyFlagBits::eHostVisible;

    auto props = pdev.getMemoryProperties();
    for (uint32_t i = 0; i < props.memoryTypeCount; i++) {
      vk::MemoryType mt = props.memoryTypes[i];
      if ((mt.propertyFlags | mem_flags) == mem_flags) {
        res.mti = i;
        res.mhs = props.memoryHeaps[i].size;
        break;
      }
    }

    return res;
  }
}  // namespace

namespace vkc {
  const char default_app_name[] = "DEFAULT_VK_APP";

  instance instance::init(
    uint32_t vk_version, const char* app_name, uint32_t app_version) {
    auto inst = init_instance(vk_version, app_name, app_version);
    auto cfg = get_device_config(inst);
    auto dev = make_device(cfg);
    auto [mti, mhs] = get_mem_info(cfg.dev);

    return instance(
      std::move(inst), std::move(cfg.dev), cfg.config, std::move(dev), mti,
      mhs);
  }

  buffer buffer::alloc(const std::shared_ptr<instance>& inst, size_t size) {
    vk::BufferCreateInfo bci(
      {}, size, vk::BufferUsageFlagBits::eStorageBuffer,
      vk::SharingMode::eExclusive, 1, &inst->m_cqfi);

    vk::Buffer buf = inst->m_dev.createBuffer(bci);
    
    vk::MemoryRequirements mrq = inst->m_dev.getBufferMemoryRequirements(buf);
    vk::MemoryAllocateInfo mai(size, inst->m_mti);
    vk::DeviceMemory dmem = inst->m_dev.allocateMemory(mai);
    
    inst->m_dev.bindBufferMemory(buf, dmem, 0);
    
    return buffer(inst, std::move(buf), size, std::move(dmem));
  }
  
  shader shader::load(const std::filesystem::path &path) {
    auto file_data = [&](){
      std::ifstream file(path, std::ios::in | std::ios::ate);
      std::vector<char> data(file.tellg());
      
      file.seekg(0, std::ios::beg);
      file.read(data.data(), data.size());
      
      return data;
    }();
    
    vk::ShaderModule sdm;
  }
}  // namespace vkc