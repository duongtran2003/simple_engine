#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <utility>
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

  static std::pair<vk::Buffer, vk::DeviceMemory>
  createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties,
               const Core::RenderContext &context);

  static vk::CommandBuffer
  beginSingleTimeCommands(const Core::RenderContext &context);
  static void endSingleTimeCommands(vk::CommandBuffer commandBuffer,
                                    const Core::RenderContext &context);

  static void copyBuffer(const vk::Buffer &src, vk::Buffer &dst,
                         vk::DeviceSize bufferSize, const Core::RenderContext& context);
};
} // namespace Helper
} // namespace SimpleEngine
