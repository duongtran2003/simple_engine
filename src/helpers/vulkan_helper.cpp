#include "helpers/vulkan_helper.hpp"
#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
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

void VulkanHelper::transitionImageLayout(vk::CommandBuffer &commandBuffer,
                                         vk::Image image,
                                         vk::ImageLayout oldLayout,
                                         vk::ImageLayout newLayout,
                                         vk::ImageAspectFlags aspectMask) {
  if (oldLayout == newLayout) {
    return;
  }

  vk::ImageMemoryBarrier2 barrier{.oldLayout = oldLayout,
                                  .newLayout = newLayout,
                                  .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
                                  .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
                                  .image = image,
                                  .subresourceRange = {.aspectMask = aspectMask,
                                                       .baseMipLevel = 0,
                                                       .levelCount = 1,
                                                       .baseArrayLayer = 0,
                                                       .layerCount = 1}};

  if (oldLayout == vk::ImageLayout::eUndefined) {
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
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests;
    barrier.srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite |
                            vk::AccessFlagBits2::eDepthStencilAttachmentRead;
  } else if (newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
    barrier.srcAccessMask = vk::AccessFlagBits2::eShaderRead;
  } else if (newLayout == vk::ImageLayout::eTransferSrcOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
  } else if (newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eAllTransfer;
    barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
  } else if (newLayout == vk::ImageLayout::ePresentSrcKHR) {
    barrier.srcStageMask = vk::PipelineStageFlagBits2::eNone;
    barrier.srcAccessMask = vk::AccessFlagBits2::eNone;
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
VulkanHelper::createImage(uint32_t width, uint32_t height, vk::Format format,
                          vk::ImageUsageFlags usage,
                          vk::ImageAspectFlags aspectMask,
                          const Core::RenderContext &context) {
  vk::ImageCreateInfo imageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = context.msaaSamples,
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
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  vk::ImageView imageView = context.device.createImageView(imageViewCreateInfo);

  return {std::move(image), std::move(imageMemory), std::move(imageView)};
}

vk::Sampler VulkanHelper::createImageSampler(
    Core::TextureFilter magFilter, Core::TextureFilter minFilter,
    Core::TextureWrapMode wrapS, Core::TextureWrapMode wrapT,
    const Core::RenderContext &context) {
  vk::PhysicalDeviceProperties deviceProperties =
      context.physicalDevice.getProperties();

  vk::Filter vkMagFilter;
  if (magFilter == Core::TextureFilter::Linear) {
    vkMagFilter = vk::Filter::eLinear;
  } else if (magFilter == Core::TextureFilter::Nearest) {
    vkMagFilter = vk::Filter::eNearest;
  }

  vk::Filter vkMinFilter;
  if (minFilter == Core::TextureFilter::Linear) {
    vkMinFilter = vk::Filter::eLinear;
  } else if (minFilter == Core::TextureFilter::Nearest) {
    vkMinFilter = vk::Filter::eNearest;
  }

  vk::SamplerAddressMode vkUWrap;
  if (wrapS == Core::TextureWrapMode::MirroredRepeat) {
    vkUWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (wrapS == Core::TextureWrapMode::Repeat) {
    vkUWrap = vk::SamplerAddressMode::eRepeat;
  } else if (wrapS == Core::TextureWrapMode::ClampToEdge) {
    vkUWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  vk::SamplerAddressMode vkVWrap;
  if (wrapT == Core::TextureWrapMode::MirroredRepeat) {
    vkVWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (wrapT == Core::TextureWrapMode::Repeat) {
    vkVWrap = vk::SamplerAddressMode::eRepeat;
  } else if (wrapT == Core::TextureWrapMode::ClampToEdge) {
    vkVWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  vk::SamplerCreateInfo samplerInfo{
      .magFilter = vkMagFilter,
      .minFilter = vkMinFilter,
      .mipmapMode = vk::SamplerMipmapMode::eLinear,
      .addressModeU = vkUWrap,
      .addressModeV = vkVWrap,
      .addressModeW = vk::SamplerAddressMode::eRepeat,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::False,
      .maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy,
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
