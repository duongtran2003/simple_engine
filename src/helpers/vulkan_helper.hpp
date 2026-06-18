#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>
namespace SimpleEngine {
namespace Helper {
class VulkanHelper {
public:
  VulkanHelper() = delete;

  static uint32_t findMemoryType(uint32_t memoryTypeBits,
                                 vk::MemoryPropertyFlags properties,
                                 const vk::PhysicalDevice &physicalDevice);
};
} // namespace Helper
} // namespace SimpleEngine
