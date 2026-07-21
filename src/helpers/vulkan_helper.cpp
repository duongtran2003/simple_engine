#include "helpers/vulkan_helper.hpp"
#include "core/render_context.hpp"
#include "enums/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vulkan/vulkan_to_string.hpp>

namespace SimpleEngine {
namespace Helper {
uint32_t
VulkanHelper::findMemoryType(uint32_t memoryTypeBits,
                             vk::MemoryPropertyFlags properties,
                             const vk::PhysicalDevice &physicalDevice) {
  vk::PhysicalDeviceMemoryProperties memoryProperties =
      physicalDevice.getMemoryProperties();
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
      return i;
    }
  }

  throw std::runtime_error(
      "VulkanHelper::findMemoryType()::ERROR: Cannot find memory type.");
}

void VulkanHelper::transitionImageLayout(
    vk::CommandBuffer &commandBuffer, vk::Image image, uint32_t mipLevels,
    uint32_t baseMipLevel, uint32_t layerCount, uint32_t baseLayer,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
    vk::ImageAspectFlags aspectMask) {
  vk::ImageMemoryBarrier2 barrier{
      .oldLayout = oldLayout,
      .newLayout = newLayout,
      .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
      .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
      .image = image,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = baseMipLevel,
                           .levelCount = mipLevels,
                           .baseArrayLayer = baseLayer,
                           .layerCount = layerCount}};

  if (oldLayout == vk::ImageLayout::eUndefined ||
      oldLayout == vk::ImageLayout::ePreinitialized) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eNone;
    barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
  } else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
  } else if (oldLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eLateFragmentTests;
    barrier.srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite;
  } else if (oldLayout == vk::ImageLayout::eTransferSrcOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
  } else {
    throw std::runtime_error(
        std::string("VulkanHelper::transitionImageLayout::ERROR: "
                    "Unsupported source layout: " +
                    vk::to_string(oldLayout)));
  }

  if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    barrier.dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite;
  } else if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
    barrier.dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite |
                            vk::AccessFlagBits2::eDepthStencilAttachmentRead;
  } else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
    barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;
  } else if (newLayout == vk::ImageLayout::eTransferSrcOptimal) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.dstAccessMask = vk::AccessFlagBits2::eTransferRead;
  } else if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.dstAccessMask = vk::AccessFlagBits2::eTransferWrite;
  } else if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
    barrier.dstStageMask = vk::PipelineStageFlagBits2::eNone;
    barrier.dstAccessMask = vk::AccessFlagBits2::eNone;
  } else {
    throw std::runtime_error(
        std::string("VulkanHelper::transitionImageLayout::ERROR: "
                    "Unsupported dest layout: " +
                    vk::to_string(newLayout)));
  }

  vk::DependencyInfo dependencyInfo{.dependencyFlags =
                                        vk::DependencyFlagBits::eByRegion,
                                    .imageMemoryBarrierCount = 1,
                                    .pImageMemoryBarriers = &barrier};

  commandBuffer.pipelineBarrier2(dependencyInfo);
}

std::pair<vk::Buffer, vk::DeviceMemory>
VulkanHelper::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                           vk::MemoryPropertyFlags properties,
                           const Core::RenderContext &context) {
  vk::Device device = context.device;
  vk::PhysicalDevice physicalDevice = context.physicalDevice;

  vk::BufferCreateInfo bufferInfo{
      .size = size, .usage = usage, .sharingMode = vk::SharingMode::eExclusive};

  vk::Buffer buffer = device.createBuffer(bufferInfo);
  vk::MemoryRequirements memoryRequirements =
      device.getBufferMemoryRequirements(buffer);

  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirements.size,
      .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,
                                        properties, physicalDevice)};

  vk::DeviceMemory bufferMemory = device.allocateMemory(allocateInfo);
  device.bindBufferMemory(buffer, bufferMemory, 0);

  return {std::move(buffer), std::move(bufferMemory)};
}

std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
VulkanHelper::createImage(uint32_t width, uint32_t height, uint32_t mipLevels,
                          uint32_t arrayLayers, vk::Format format,
                          vk::ImageUsageFlags usage,
                          vk::ImageAspectFlags aspectMask,
                          vk::SampleCountFlagBits sampleCount,
                          const Core::RenderContext &context) {
  vk::ImageCreateInfo imageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = mipLevels,
      .arrayLayers = arrayLayers,
      .samples = sampleCount,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  vk::Image image = context.device.createImage(imageCreateInfo);

  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(image, &memoryRequirement);
  vk::MemoryAllocateInfo imageMemoryAllocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};

  vk::DeviceMemory imageMemory =
      context.device.allocateMemory(imageMemoryAllocateInfo);
  context.device.bindImageMemory(image, imageMemory, 0);

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = mipLevels,
                           .baseArrayLayer = 0,
                           .layerCount = arrayLayers}};
  vk::ImageView imageView = context.device.createImageView(imageViewCreateInfo);

  return {std::move(image), std::move(imageMemory), std::move(imageView)};
}

std::tuple<vk::Image, vk::DeviceMemory, vk::ImageView>
VulkanHelper::createCubemapImage(uint32_t dimension, uint32_t mipLevels,
                                 vk::Format format, vk::ImageUsageFlags usage,
                                 vk::ImageAspectFlags aspectMask,
                                 const Core::RenderContext &context) {
  vk::ImageCreateInfo imageCreateInfo{
      .flags = vk::ImageCreateFlagBits::eCubeCompatible,
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {dimension, dimension, 1},
      .mipLevels = mipLevels,
      .arrayLayers = 6,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = usage,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  vk::Image image = context.device.createImage(imageCreateInfo);

  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(image, &memoryRequirement);
  vk::MemoryAllocateInfo imageMemoryAllocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};

  vk::DeviceMemory imageMemory =
      context.device.allocateMemory(imageMemoryAllocateInfo);
  context.device.bindImageMemory(image, imageMemory, 0);

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .image = image,
      .viewType = vk::ImageViewType::eCube,
      .format = format,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = mipLevels,
                           .baseArrayLayer = 0,
                           .layerCount = 6}};
  vk::ImageView imageView = context.device.createImageView(imageViewCreateInfo);

  return {std::move(image), std::move(imageMemory), std::move(imageView)};
}

vk::Sampler VulkanHelper::createImageSampler(
    Enums::Texture::Filter magFilter, Enums::Texture::Filter minFilter,
    Enums::Texture::Wrap wrapS, Enums::Texture::Wrap wrapT,
    const Core::RenderContext &context) {
  vk::PhysicalDeviceProperties deviceProperties =
      context.physicalDevice.getProperties();

  vk::Filter vkMagFilter;
  if (magFilter == Enums::Texture::Filter::Linear) {
    vkMagFilter = vk::Filter::eLinear;
  } else {
    vkMagFilter = vk::Filter::eNearest;
  }

  vk::Filter vkMinFilter;
  if (minFilter == Enums::Texture::Filter::Linear) {
    vkMinFilter = vk::Filter::eLinear;
  } else {
    vkMinFilter = vk::Filter::eNearest;
  }

  vk::SamplerAddressMode vkUWrap;
  if (wrapS == Enums::Texture::Wrap::MirroredRepeat) {
    vkUWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (wrapS == Enums::Texture::Wrap::Repeat) {
    vkUWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkUWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  vk::SamplerAddressMode vkVWrap;
  if (wrapT == Enums::Texture::Wrap::MirroredRepeat) {
    vkVWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (wrapT == Enums::Texture::Wrap::Repeat) {
    vkVWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkVWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vkMagFilter,
      .minFilter = vkMinFilter,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vkUWrap,
      .addressModeV = vkVWrap,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = -0.5f,
      .anisotropyEnable = vk::True,
      .maxAnisotropy =
          std::min(16.0f, deviceProperties.limits.maxSamplerAnisotropy),
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0f,
      .maxLod = vk::LodClampNone,
      .borderColor = vk::BorderColor::eIntOpaqueBlack,
      .unnormalizedCoordinates = vk::False};
  vk::Sampler sampler = context.device.createSampler(samplerInfo);

  return std::move(sampler);
}

vk::CommandBuffer
VulkanHelper::beginSingleTimeCommands(const Core::RenderContext &context) {
  vk::CommandBufferAllocateInfo allocateInfo{
      .commandPool = context.commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = 1};

  vk::CommandBuffer commandBuffer =
      context.device.allocateCommandBuffers(allocateInfo).front();

  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  return commandBuffer;
}

void VulkanHelper::endSingleTimeCommands(vk::CommandBuffer commandBuffer,
                                         const Core::RenderContext &context) {
  commandBuffer.end();
  vk::SubmitInfo submitInfo{.commandBufferCount = 1,
                            .pCommandBuffers = &commandBuffer};
  context.graphicsQueue.submit(submitInfo, nullptr);
  context.graphicsQueue.waitIdle();

  context.device.freeCommandBuffers(context.commandPool, 1, &commandBuffer);
}

void VulkanHelper::copyBuffer(const vk::Buffer &src, vk::Buffer &dst,
                              vk::DeviceSize bufferSize,
                              const Core::RenderContext &context) {
  vk::CommandBuffer commandBuffer = beginSingleTimeCommands(context);
  commandBuffer.copyBuffer(src, dst, vk::BufferCopy(0, 0, bufferSize));
  endSingleTimeCommands(commandBuffer, context);
}

void VulkanHelper::copyBufferToImage(vk::CommandBuffer &commandBuffer,
                                     vk::Buffer &src, vk::Image &dst,
                                     uint32_t width, uint32_t height,
                                     vk::ImageAspectFlags aspectMask) {
  vk::BufferImageCopy region{
      .bufferOffset = 0,
      .bufferRowLength = 0,
      .bufferImageHeight = 0,
      .imageSubresource = {aspectMask, 0, 0, 1},
      .imageOffset = {0, 0, 0},
      .imageExtent = {.width = width, .height = height, .depth = 1}};

  commandBuffer.copyBufferToImage(src, dst,
                                  vk::ImageLayout::eTransferDstOptimal, region);
}
} // namespace Helper
} // namespace SimpleEngine
