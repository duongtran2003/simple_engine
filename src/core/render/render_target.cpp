#include "core/render/render_target.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
RenderTarget::RenderTarget(uint32_t w, uint32_t h,
                           const RenderContext &rContext)
    : width(w), height(h), context(rContext) {
  createColorResources();
  createDepthResources();
}

RenderTarget::~RenderTarget() {
  if (colorImageView) {
    context.device.destroyImageView(colorImageView);
    colorImageView = nullptr;
  }
  if (colorImage) {
    context.device.destroyImage(colorImage);
    colorImage = nullptr;
  }
  if (colorMemory) {
    context.device.freeMemory(colorMemory);
    colorMemory = nullptr;
  }

  if (depthImageView) {
    context.device.destroyImageView(depthImageView);
    depthImageView = nullptr;
  }
  if (depthImage) {
    context.device.destroyImage(depthImage);
    depthImage = nullptr;
  }
  if (depthMemory) {
    context.device.freeMemory(depthMemory);
    depthMemory = nullptr;
  }
}

vk::ImageView RenderTarget::getColorImageView() const { return colorImageView; }
vk::ImageView RenderTarget::getDepthImageView() const { return depthImageView; }

vk::Image RenderTarget::getColorImage() const { return colorImage; }
vk::Image RenderTarget::getDepthImage() const { return depthImage; }

vk::ImageLayout RenderTarget::getColorLayout() const { return colorLayout; }
vk::ImageLayout RenderTarget::getDepthLayout() const { return depthLayout; }

uint32_t RenderTarget::getWidth() const { return width; }
uint32_t RenderTarget::getHeight() const { return height; }

void RenderTarget::transitionColorLayout(vk::CommandBuffer &commandBuffer,
                                         vk::ImageLayout dstLayout) {
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, colorImage,
                                              colorLayout, dstLayout,
                                              vk::ImageAspectFlagBits::eColor);
  colorLayout = dstLayout;
}

void RenderTarget::transitionDepthLayout(vk::CommandBuffer &commandBuffer,
                                         vk::ImageLayout dstLayout) {
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, depthImage,
                                              depthLayout, dstLayout,
                                              vk::ImageAspectFlagBits::eDepth);
  depthLayout = dstLayout;
}

void RenderTarget::createColorResources() {
  vk::Format colorFormat = context.swapChainSurfaceFormat.format;

  vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                .format = colorFormat,
                                .extent = {width, height, 1},
                                .mipLevels = 1,
                                .arrayLayers = 1,
                                .samples = vk::SampleCountFlagBits::e1,
                                .tiling = vk::ImageTiling::eOptimal,
                                .usage =
                                    vk::ImageUsageFlagBits::eColorAttachment |
                                    vk::ImageUsageFlagBits::eSampled |
                                    vk::ImageUsageFlagBits::eTransferSrc,
                                .sharingMode = vk::SharingMode::eExclusive,
                                .initialLayout = vk::ImageLayout::eUndefined};
  colorImage = context.device.createImage(imageInfo);

  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(colorImage, &memoryRequirement);
  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};
  colorMemory = context.device.allocateMemory(allocateInfo);
  context.device.bindImageMemory(colorImage, colorMemory, 0);

  vk::ImageViewCreateInfo viewInfo{
      .image = colorImage,
      .viewType = vk::ImageViewType::e2D,
      .format = colorFormat,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  colorImageView = context.device.createImageView(viewInfo);
}

void RenderTarget::createDepthResources() {
  vk::Format depthFormat = vk::Format::eD32Sfloat;

  vk::ImageCreateInfo imageInfo{
      .imageType = vk::ImageType::e2D,
      .format = depthFormat,
      .extent = {width, height, 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment |
               vk::ImageUsageFlagBits::eSampled |
               vk::ImageUsageFlagBits::eTransferSrc,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};
  depthImage = context.device.createImage(imageInfo);

  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(depthImage, &memoryRequirement);
  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};
  depthMemory = context.device.allocateMemory(allocateInfo);
  context.device.bindImageMemory(depthImage, depthMemory, 0);

  vk::ImageViewCreateInfo viewInfo{
      .image = depthImage,
      .viewType = vk::ImageViewType::e2D,
      .format = depthFormat,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eDepth,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  depthImageView = context.device.createImageView(viewInfo);
}
} // namespace Core
} // namespace SimpleEngine
