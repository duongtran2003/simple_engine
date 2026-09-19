#include "core/image/image.hpp"
#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>

namespace SimpleEngine {
namespace Core {
Image::Image(const CreateInfo &createInfo, const RenderContext *context) {
  m_type = createInfo.type;
  m_format = createInfo.format;
  m_extent = createInfo.extent;
  m_mipLevels = createInfo.mipLevels;
  m_arrayLayers = createInfo.arrayLayers;
  m_sampleCount = createInfo.sampleCount;
  m_tiling = createInfo.tiling;
  m_usage = createInfo.usage;
  m_sharingMode = createInfo.sharingMode;
  m_aspectMask = createInfo.aspectMask;

  m_width = createInfo.width;
  m_height = createInfo.height;
  m_channels = createInfo.channels;

  m_colorSpace = createInfo.colorSpace;

  m_pContext = context;

  Create();
}

Image::~Image() {};

void Image::Create() {
  CreateImage();
  AllocMemory();
  CreateView();
  m_isValid = true;
}

void Image::Free() {
  if (!m_isValid) {
    return;
  }

  m_pContext->device.destroyImageView(m_view);
  m_pContext->device.freeMemory(m_memory);
  m_pContext->device.destroyImage(m_image);

  m_isValid = false;
}

void Image::CreateImage() {
  vk::ImageCreateInfo info{.imageType = m_type,
                           .format = m_format,
                           .extent = m_extent,
                           .mipLevels = m_mipLevels,
                           .arrayLayers = m_arrayLayers,
                           .samples = m_sampleCount,
                           .tiling = m_tiling,
                           .usage = m_usage,
                           .sharingMode = m_sharingMode,
                           .initialLayout = vk::ImageLayout::eUndefined};

  m_image = m_pContext->device.createImage(info);
}

void Image::AllocMemory() {
  vk::MemoryRequirements memReq;
  m_pContext->device.getImageMemoryRequirements(m_image, &memReq);
  uint32_t memTypeIdx = Helper::VulkanHelper::findMemoryType(
      memReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal,
      m_pContext->physicalDevice);

  vk::MemoryAllocateInfo info{.allocationSize = memReq.size,
                              .memoryTypeIndex = memTypeIdx};

  m_memory = m_pContext->device.allocateMemory(info);
  m_pContext->device.bindImageMemory(m_image, m_memory, 0);
}

void Image::CreateView() {
  vk::ImageViewType viewType = vk::ImageViewType::e2D;
  if (m_type == vk::ImageType::e1D) {
    viewType = vk::ImageViewType::e1D;
  } else if (m_type == vk::ImageType::e2D) {
    viewType = vk::ImageViewType::e2D;
  } else if (m_type == vk::ImageType::e3D) {
    viewType = vk::ImageViewType::e3D;
  };

  vk::ImageViewCreateInfo info{
      .viewType = viewType,
      .format = m_format,
      .subresourceRange = {.aspectMask = m_aspectMask,
                           .baseMipLevel = 0,
                           .levelCount = m_mipLevels,
                           .baseArrayLayer = 0,
                           .layerCount = m_arrayLayers}};

  m_view = m_pContext->device.createImageView(info);
}

void Image::TransitionLayout(vk::ImageLayout toLayout, bool keepContent,
                             vk::CommandBuffer &commandBuffer) {
  if (!m_isValid) {
    return;
  }

  vk::ImageLayout oldLayout =
      keepContent ? m_layout : vk::ImageLayout::eUndefined;
  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, m_image, m_mipLevels, 0, m_arrayLayers, 0, oldLayout,
      toLayout, m_aspectMask);

  m_layout = toLayout;
}

bool Image::IsValid() const { return m_isValid; }
} // namespace Core
} // namespace SimpleEngine
