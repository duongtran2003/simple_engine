#include "core/render_graph/graph_resource.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
GraphResource::GraphResource(const std::string &name, uint32_t width,
                             uint32_t height, vk::Format format,
                             vk::ImageLayout layout,
                             vk::ImageAspectFlags aspectMask,
                             vk::ImageUsageFlags usage,
                             const RenderContext &context)
    : context(context), name(name), format(format), layout(layout),
      aspectMask(aspectMask), usage(usage), width(width), height(height) {
  createImage();
  allocateMemory();
  createView();
}

GraphResource::~GraphResource() {
  destroyView();
  destroyImage();
  deallocateMemory();
}

const std::string &GraphResource::getName() const { return name; }
vk::Image GraphResource::getImage() { return image; }
vk::ImageView GraphResource::getView() { return view; }
vk::DeviceMemory GraphResource::getMemory() { return memory; }
vk::Format GraphResource::getFormat() { return format; }
vk::ImageLayout GraphResource::getLayout() { return layout; }
vk::ImageAspectFlags GraphResource::getAspectMask() { return aspectMask; }

uint32_t GraphResource::getWidth() const { return width; }

uint32_t GraphResource::getHeight() const { return height; }

void GraphResource::createImage() {
  vk::ImageCreateInfo createInfo{.imageType = vk::ImageType::e2D,
                                 .format = format,
                                 .extent = {width, height, 1},
                                 .mipLevels = 1,
                                 .arrayLayers = 1,
                                 .samples = vk::SampleCountFlagBits::e1,
                                 .tiling = vk::ImageTiling::eOptimal,
                                 .usage = usage,
                                 .sharingMode = vk::SharingMode::eExclusive,
                                 .initialLayout = vk::ImageLayout::eUndefined};

  image = context.device.createImage(createInfo);
}

void GraphResource::allocateMemory() {
  vk::MemoryRequirements memoryRequirement;
  context.device.getImageMemoryRequirements(image, &memoryRequirement);

  vk::MemoryAllocateInfo allocateInfo{
      .allocationSize = memoryRequirement.size,
      .memoryTypeIndex = Helper::VulkanHelper::findMemoryType(
          memoryRequirement.memoryTypeBits,
          vk::MemoryPropertyFlagBits::eDeviceLocal, context.physicalDevice)};

  memory = context.device.allocateMemory(allocateInfo);
  context.device.bindImageMemory(image, memory, 0);
}

void GraphResource::createView() {
  vk::ImageViewCreateInfo createInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  view = context.device.createImageView(createInfo);
}

void GraphResource::destroyImage() {
  context.device.destroyImage(image);
  image = nullptr;
}

void GraphResource::destroyView() {
  context.device.destroyImageView(view);
  view = nullptr;
}

void GraphResource::deallocateMemory() {
  context.device.freeMemory(memory);
  memory = nullptr;
}

void GraphResource::transitionLayout(vk::CommandBuffer &commandBuffer,
                                     vk::ImageLayout dstLayout) {
  Helper::VulkanHelper::transitionImageLayout(commandBuffer, image, layout,
                                              dstLayout, aspectMask);
  layout = dstLayout;
}
} // namespace Core
} // namespace SimpleEngine
