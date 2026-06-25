#pragma once

#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <tuple>
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

  static std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
  createImage(uint32_t width, uint32_t height, vk::Format format,
              vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectMask,
              const Core::RenderContext &context);

  static vk::Sampler createImageSampler(Core::TextureFilter magFilter,
                                        Core::TextureFilter minFilter,
                                        Core::TextureWrapMode wrapS,
                                        Core::TextureWrapMode wrapT,
                                        const Core::RenderContext &context);

  static vk::CommandBuffer
  beginSingleTimeCommands(const Core::RenderContext &context);
  static void endSingleTimeCommands(vk::CommandBuffer commandBuffer,
                                    const Core::RenderContext &context);

  static void copyBuffer(const vk::Buffer &src, vk::Buffer &dst,
                         vk::DeviceSize bufferSize,
                         const Core::RenderContext &context);

  static void copyBufferToImage(vk::CommandBuffer &commandBuffer,
                                vk::Buffer &src, vk::Image &dst, uint32_t width,
                                uint32_t height, vk::ImageAspectFlags aspectMask);
};
} // namespace Helper
} // namespace SimpleEngine
