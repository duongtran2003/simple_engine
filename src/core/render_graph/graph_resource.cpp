#include "core/render_graph/graph_resource.hpp"
#include "core/render_context.hpp"
#include "enums/texture.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
GraphResource::GraphResource(
    const std::string &name, uint32_t width, uint32_t height, vk::Format format,
    vk::ImageLayout layout, vk::ImageAspectFlags aspectMask,
    vk::ImageUsageFlags usage, vk::SampleCountFlagBits sampleCount,
    uint32_t inFlightFrame, const RenderContext &context)
    : context(context), inFlightFrame(inFlightFrame), name(name),
      format(format), aspectMask(aspectMask), usage(usage),
      sampleCount(sampleCount), width(width), height(height) {
  initiateLayouts(layout);
  createImages();
  allocateMemories();
  createViews();
  createSampler();
}

GraphResource::~GraphResource() {
  destroySampler();
  destroyViews();
  destroyImages();
  deallocateMemories();
}

const std::string &GraphResource::getName() const { return name; }
vk::Image GraphResource::getImage(uint32_t frameIndex) {
  return images[frameIndex];
}
vk::ImageView GraphResource::getView(uint32_t frameIndex) {
  return views[frameIndex];
}
vk::DeviceMemory GraphResource::getMemory(uint32_t frameIndex) {
  return memories[frameIndex];
}
vk::Format GraphResource::getFormat() { return format; }
vk::ImageLayout GraphResource::getLayout(uint32_t frameIndex) {
  return layouts[frameIndex];
}
vk::ImageAspectFlags GraphResource::getAspectMask() { return aspectMask; }
vk::Sampler GraphResource::getSampler() { return sampler; }

uint32_t GraphResource::getWidth() const { return width; }

uint32_t GraphResource::getHeight() const { return height; }

GraphResource *
GraphResource::bindSlot(std::vector<vk::DescriptorSet> &descriptorSets,
                        uint32_t slot) {
  assert(descriptorSets.size() == views.size());

  bindingSlot = slot;

  std::vector<vk::WriteDescriptorSet> writes;
  for (size_t i = 0; i < descriptorSets.size(); i++) {
    vk::DescriptorImageInfo imageInfo{
        .sampler = sampler,
        .imageView = views[i],
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = bindingSlot,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo};

    writes.push_back(descriptorWrite);
  }

  context.device.updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
  return this;
}

uint32_t GraphResource::getBindSlot() const { return bindingSlot; }

void GraphResource::initiateLayouts(vk::ImageLayout layout) {
  layouts.assign(4, layout);
}

void GraphResource::createImages() {
  vk::ImageCreateInfo createInfo{.imageType = vk::ImageType::e2D,
                                 .format = format,
                                 .extent = {width, height, 1},
                                 .mipLevels = 1,
                                 .arrayLayers = 1,
                                 .samples = sampleCount,
                                 .tiling = vk::ImageTiling::eOptimal,
                                 .usage = usage,
                                 .sharingMode = vk::SharingMode::eExclusive,
                                 .initialLayout = vk::ImageLayout::eUndefined};

  images.resize(inFlightFrame);
  for (uint32_t i = 0; i < inFlightFrame; i++) {
    images[i] = context.device.createImage(createInfo);
  }
}

void GraphResource::createSampler() {
  sampler = Helper::VulkanHelper::createImageSampler(
      Enums::Texture::Filter::Nearest, Enums::Texture::Filter::Nearest,
      Enums::Texture::Wrap::ClampToEdge, Enums::Texture::Wrap::ClampToEdge,
      context);
}

void GraphResource::allocateMemories() {
  assert(images.size() > 0);
  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(images[0], &memoryRequirement);

  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};

  memories.resize(inFlightFrame);
  for (uint32_t i = 0; i < inFlightFrame; i++) {
    memories[i] = context.device.allocateMemory(allocateInfo);
    context.device.bindImageMemory(images[i], memories[i], 0);
  }
}

void GraphResource::createViews() {
  vk::ImageViewCreateInfo createInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  views.resize(inFlightFrame);
  for (uint32_t i = 0; i < inFlightFrame; i++) {
    createInfo.image = images[i];
    views[i] = context.device.createImageView(createInfo);
  }
}

void GraphResource::destroySampler() {
  context.device.destroySampler(sampler);
  sampler = nullptr;
}

void GraphResource::destroyImages() {
  for (uint32_t i = 0; i < images.size(); i++) {
    context.device.destroyImage(images[i]);
  }

  images.clear();
  images.shrink_to_fit();
}

void GraphResource::destroyViews() {
  for (uint32_t i = 0; i < views.size(); i++) {
    context.device.destroyImageView(views[i]);
  }

  views.clear();
  views.shrink_to_fit();
}

void GraphResource::deallocateMemories() {
  for (uint32_t i = 0; i < memories.size(); i++) {
    context.device.freeMemory(memories[i]);
  }

  memories.clear();
  memories.shrink_to_fit();
}

void GraphResource::transitionLayout(vk::CommandBuffer &commandBuffer,
                                     uint32_t frameIndex,
                                     vk::ImageLayout dstLayout,
                                     bool fromLastLayout) {
  vk::ImageLayout lastLayout =
      fromLastLayout ? layouts[frameIndex] : vk::ImageLayout::eUndefined;
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, images[frameIndex],
                                              1, 0, 1, 0, lastLayout, dstLayout,
                                              aspectMask);
  layouts[frameIndex] = dstLayout;
}
} // namespace Core
} // namespace SimpleEngine
