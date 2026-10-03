#include "core/image/image.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
Image::Image(const CreateInfo &createInfo, const RenderContext *context) {
  m_Type = createInfo.Type;
  m_Format = createInfo.Format;
  m_Extent = createInfo.Extent;
  m_MipLevels = createInfo.MipLevels;
  m_ArrayLayers = createInfo.ArrayLayers;
  m_SampleCount = createInfo.SampleCount;
  m_Tiling = createInfo.Tiling;
  m_Usage = createInfo.Usage;
  m_SharingMode = createInfo.SharingMode;
  m_AspectMask = createInfo.AspectMask;

  m_Width = createInfo.Width;
  m_Height = createInfo.Height;
  m_Channels = createInfo.Channels;

  m_ColorSpace = createInfo.ColorSpace;

  m_pContext = context;

  Create();
}

Image::~Image() {
  // TODO: Destructor
};

void Image::Create() {
  CreateImage();
  AllocMemory();
  CreateView();
  m_IsValid = true;
}

void Image::Free() {
  if (!m_IsValid) {
    return;
  }

  m_pContext->device.destroyImageView(m_View);
  m_pContext->device.freeMemory(m_Memory);
  m_pContext->device.destroyImage(m_Image);

  m_IsValid = false;
}

void Image::CreateImage() {
  vk::ImageCreateInfo info{.imageType = m_Type,
                           .format = m_Format,
                           .extent = m_Extent,
                           .mipLevels = m_MipLevels,
                           .arrayLayers = m_ArrayLayers,
                           .samples = m_SampleCount,
                           .tiling = m_Tiling,
                           .usage = m_Usage,
                           .sharingMode = m_SharingMode,
                           .initialLayout = vk::ImageLayout::eUndefined};

  m_Image = m_pContext->device.createImage(info);
}

void Image::AllocMemory() {
  vk::MemoryRequirements memReq;
  m_pContext->device.getImageMemoryRequirements(m_Image, &memReq);
  uint32_t memTypeIdx = Helper::VulkanHelper::findMemoryType(
      memReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal,
      m_pContext->physicalDevice);

  vk::MemoryAllocateInfo info{.allocationSize = memReq.size,
                              .memoryTypeIndex = memTypeIdx};

  m_Memory = m_pContext->device.allocateMemory(info);
  m_pContext->device.bindImageMemory(m_Image, m_Memory, 0);
}

void Image::CreateView() {
  vk::ImageViewType viewType = vk::ImageViewType::e2D;
  if (m_Type == vk::ImageType::e1D) {
    viewType = vk::ImageViewType::e1D;
  } else if (m_Type == vk::ImageType::e2D) {
    viewType = vk::ImageViewType::e2D;
  } else if (m_Type == vk::ImageType::e3D) {
    viewType = vk::ImageViewType::e3D;
  };

  vk::ImageViewCreateInfo info{
      .viewType = viewType,
      .format = m_Format,
      .subresourceRange = {.aspectMask = m_AspectMask,
                           .baseMipLevel = 0,
                           .levelCount = m_MipLevels,
                           .baseArrayLayer = 0,
                           .layerCount = m_ArrayLayers}};

  m_View = m_pContext->device.createImageView(info);
}

void Image::TransitionLayout(vk::ImageLayout toLayout, bool keepContent,
                             vk::CommandBuffer &commandBuffer) {
  if (!m_IsValid) {
    return;
  }

  vk::ImageLayout oldLayout =
      keepContent ? m_Layout : vk::ImageLayout::eUndefined;
  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, m_Image, m_MipLevels, 0, m_ArrayLayers, 0, oldLayout,
      toLayout, m_AspectMask);

  m_Layout = toLayout;
}

bool Image::IsValid() const { return m_IsValid; }
} // namespace Core
} // namespace SimpleEngine
