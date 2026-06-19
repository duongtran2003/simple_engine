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

  static void transitionImageLayout(vk::CommandBuffer &commandBuffer,
                                    vk::Image image, vk::ImageLayout oldLayout,
                                    vk::ImageLayout newLayout,
                                    vk::ImageAspectFlags aspectMask);
};
} // namespace Helper
} // namespace SimpleEngine
