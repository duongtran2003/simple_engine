#include "core/image/image.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <cstring>

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
vk::Image Image::GetImage() const { return m_Image; }
vk::ImageView Image::GetView() const { return m_View; }
vk::Format Image::GetFormat() const { return m_Format; }
vk::Extent3D Image::GetExtent() const { return m_Extent; };
vk::ImageLayout Image::GetLayout() const { return m_Layout; }

void Image::UploadData(const void *pixels, vk::DeviceSize size) {
  auto [stagingBuffer, stagingMemory] = Helper::VulkanHelper::createBuffer(
      size, vk::BufferUsageFlagBits::eTransferSrc,
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent,
      *m_pContext);

  void *data = m_pContext->device.mapMemory(stagingMemory, 0, size);
  memcpy(data, pixels, static_cast<size_t>(size));
  m_pContext->device.unmapMemory(stagingMemory);

  vk::CommandBuffer commandBuffer =
      Helper::VulkanHelper::beginSingleTimeCommands(*m_pContext);

  TransitionLayout(vk::ImageLayout::eTransferDstOptimal, false, commandBuffer);
  Helper::VulkanHelper::copyBufferToImage(commandBuffer, stagingBuffer, m_Image,
                                          m_Extent.width, m_Extent.height,
                                          vk::ImageAspectFlagBits::eColor);
  TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal, true,
                   commandBuffer);
  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, *m_pContext);
  m_pContext->device.destroyBuffer(stagingBuffer);
  m_pContext->device.freeMemory(stagingMemory);
}
} // namespace Core
} // namespace SimpleEngine
