#pragma once

#include "enums/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <tuple>
#include <utility>
namespace SimpleEngine {

namespace Core {
class RenderContext;
}

namespace Helper {
class VulkanHelper {
public:
  VulkanHelper() = delete;

  static uint32_t findMemoryType(uint32_t memoryTypeBits,
                                 vk::MemoryPropertyFlags properties,
                                 const vk::PhysicalDevice &physicalDevice);

  static void transitionImageLayout(vk::CommandBuffer &commandBuffer,
                                    vk::Image image, uint32_t mipLevels,
                                    uint32_t baseMipLevel, uint32_t layerCount,
                                    uint32_t baseLayer,
                                    vk::ImageLayout oldLayout,
                                    vk::ImageLayout newLayout,
                                    vk::ImageAspectFlags aspectMask);

  static std::pair<vk::Buffer, vk::DeviceMemory>
  createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties,
               const Core::RenderContext &context);

  static std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
  createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
              uint32_t arrayLayers, vk::Format format,
              vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectMask,
              vk::SampleCountFlagBits sampleCount,
              const Core::RenderContext &context);

  static std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
  createCubemapImage(uint32_t dimension, uint32_t mipLevels, vk::Format format,
                     vk::ImageUsageFlags usage, vk::ImageAspectFlags aspectMask,
                     const Core::RenderContext &context);

  static vk::Sampler createImageSampler(Enums::Texture::Filter magFilter,
                                        Enums::Texture::Filter minFilter,
                                        Enums::Texture::Wrap wrapS,
                                        Enums::Texture::Wrap wrapT,
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
                                uint32_t height,
                                vk::ImageAspectFlags aspectMask);
};
} // namespace Helper
} // namespace SimpleEngine
