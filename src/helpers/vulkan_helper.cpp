#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
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
} // namespace Helper
} // namespace SimpleEngine
