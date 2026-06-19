#include "core/render/render_target.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderTarget::RenderTarget(uint32_t w, uint32_t h,
                           std::vector<vk::Format> &colorFormats,
                           const RenderContext &rContext)
    : width(w), height(h), context(rContext) {
  size_t colorAttachmentsNum = colorFormats.size();

  this->colorFormats = colorFormats;
  colorImages.resize(colorAttachmentsNum);
  colorMemories.resize(colorAttachmentsNum);
  colorImageViews.resize(colorAttachmentsNum);
  colorLayouts.resize(colorAttachmentsNum);

  createColorResources();
  createDepthResources();
}

RenderTarget::~RenderTarget() {
  if (colorImageViews.size()) {
    for (const auto &colorImageView : colorImageViews) {
      context.device.destroyImageView(colorImageView);
    }

    colorImageViews.clear();
  }
  if (colorImages.size()) {
    for (const auto &colorImage : colorImages) {
      context.device.destroyImage(colorImage);
    }

    colorImages.clear();
  }
  if (colorMemories.size()) {
    for (const auto &colorMemory : colorMemories) {
      context.device.freeMemory(colorMemory);
    }

    colorMemories.clear();
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

const std::vector<vk::ImageView> &RenderTarget::getColorImageViews() const {
  return colorImageViews;
}
vk::ImageView RenderTarget::getColorImageView(size_t index) const {
  if (index > colorImageViews.size()) {
    throw std::runtime_error(
        "RenderTarget::getColorImageView::ERROR: Out of bound: " +
        std::to_string(index));
  }
  return colorImageViews[index];
}

vk::ImageView RenderTarget::getDepthImageView() const { return depthImageView; }

const std::vector<vk::Image> &RenderTarget::getColorImages() const {
  return colorImages;
}
vk::Image RenderTarget::getColorImage(size_t index) const {
  if (index > colorImages.size()) {
    throw std::runtime_error(
        "RenderTarget::getColorImage::ERROR: Out of bound: " +
        std::to_string(index));
  }
  return colorImages[index];
}

vk::Image RenderTarget::getDepthImage() const { return depthImage; }

const std::vector<vk::ImageLayout> &RenderTarget::getColorLayouts() const {
  return colorLayouts;
}
vk::ImageLayout RenderTarget::getColorLayout(size_t index) const {
  if (index > colorLayouts.size()) {
    throw std::runtime_error(
        "RenderTarget::getColorLayout::ERROR: Out of bound: " +
        std::to_string(index));
  }
  return colorLayouts[index];
}

vk::ImageLayout RenderTarget::getDepthLayout() const { return depthLayout; }

uint32_t RenderTarget::getWidth() const { return width; }
uint32_t RenderTarget::getHeight() const { return height; }

void RenderTarget::transitionColorLayout(vk::CommandBuffer &commandBuffer,
                                         size_t index,
                                         vk::ImageLayout dstLayout) {
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, colorImages[index],
                                              colorLayouts[index], dstLayout,
                                              vk::ImageAspectFlagBits::eColor);
  colorLayouts[index] = dstLayout;
}

void RenderTarget::transitionDepthLayout(vk::CommandBuffer &commandBuffer,
                                         vk::ImageLayout dstLayout) {
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, depthImage,
                                              depthLayout, dstLayout,
                                              vk::ImageAspectFlagBits::eDepth);
  depthLayout = dstLayout;
}

void RenderTarget::createColorResources() {
  size_t colorAttachmentNums = this->colorFormats.size();
  for (size_t i = 0; i < colorAttachmentNums; i++) {
    vk::ImageCreateInfo imageInfo{.imageType = vk::ImageType::e2D,
                                  .format = colorFormats[i],
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
    colorImages[i] = context.device.createImage(imageInfo);

    vk::MemoryRequirements memoryRequirement;
    context.device.getImageMemoryRequirements(colorImages[i],
                                              &memoryRequirement);
    vk::MemoryAllocateInfo allocateInfo{
        .allocationSize = memoryRequirement.size,
        .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
            memoryRequirement.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};
    colorMemories[i] = context.device.allocateMemory(allocateInfo);
    context.device.bindImageMemory(colorImages[i], colorMemories[i], 0);

    vk::ImageViewCreateInfo viewInfo{
        .image = colorImages[i],
        .viewType = vk::ImageViewType::e2D,
        .format = colorFormats[i],
        .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};
    colorImageViews[i] = context.device.createImageView(viewInfo);
  }
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
