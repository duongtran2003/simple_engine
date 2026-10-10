#include "core/image/image.hpp"
#include "core/buffer/buffer.hpp"
#include "core/render_context.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <stdexcept>

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

Image::~Image() { Free(); };

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
  m_Image = nullptr;
  m_View = nullptr;
  m_Memory = nullptr;
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

void Image::TransitionMipLayout(vk::ImageLayout oldLayout,
                                vk::ImageLayout toLayout, uint32_t mipLevel,
                                vk::CommandBuffer &commandBuffer) {
  if (!m_IsValid) {
    return;
  }

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, m_Image, 1, mipLevel, m_ArrayLayers, 0, oldLayout,
      toLayout, m_AspectMask);
}

bool Image::IsValid() const { return m_IsValid; }
vk::Image Image::GetImage() const { return m_Image; }
vk::ImageView Image::GetView() const { return m_View; }
vk::Format Image::GetFormat() const { return m_Format; }
vk::Extent3D Image::GetExtent() const { return m_Extent; };
vk::ImageLayout Image::GetLayout() const { return m_Layout; }

void Image::UploadData(const void *pixels, vk::DeviceSize size) {
  Buffer::CreateInfo bufferCreateInfo{
      .Size = size,
      .Usage = vk::BufferUsageFlagBits::eTransferSrc,
      .Properties = vk::MemoryPropertyFlagBits::eHostVisible |
                    vk::MemoryPropertyFlagBits::eHostCoherent};
  Buffer stagingBuffer = Buffer(bufferCreateInfo, m_pContext);
  stagingBuffer.Write(pixels);

  vk::CommandBuffer commandBuffer =
      Helper::VulkanHelper::beginSingleTimeCommands(*m_pContext);

  TransitionLayout(vk::ImageLayout::eTransferDstOptimal, false, commandBuffer);
  Helper::VulkanHelper::copyBufferToImage(commandBuffer, stagingBuffer.GetBuffer(), m_Image,
                                          m_Extent.width, m_Extent.height,
                                          vk::ImageAspectFlagBits::eColor);

  if (m_MipLevels > 1) {
    GenerateMipmap(commandBuffer);
  } else {
    TransitionLayout(vk::ImageLayout::eShaderReadOnlyOptimal, true,
                     commandBuffer);
  }

  Helper::VulkanHelper::endSingleTimeCommands(commandBuffer, *m_pContext);
}

void Image::GenerateMipmap(vk::CommandBuffer &commandBuffer) {
  vk::FormatProperties formatProperties =
      m_pContext->physicalDevice.getFormatProperties(m_Format);
  if (!(formatProperties.optimalTilingFeatures &
        vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
    throw std::runtime_error(
        "Image::GenerateMipmap::ERROR: Format is not supported.");
  }

  int32_t mipHeight = static_cast<int32_t>(m_Height);
  int32_t mipWidth = static_cast<int32_t>(m_Width);
  for (uint32_t i = 1; i < m_MipLevels; i++) {
    TransitionMipLayout(vk::ImageLayout::eTransferDstOptimal,
                        vk::ImageLayout::eTransferSrcOptimal, i - 1,
                        commandBuffer);

    vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets, dstOffsets;

    srcOffsets[0] = vk::Offset3D(0, 0, 0);
    srcOffsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);

    dstOffsets[0] = vk::Offset3D(0, 0, 0);
    dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1,
                                 mipHeight > 1 ? mipHeight / 2 : 1, 1);

    vk::ImageSubresourceLayers blitSrcSubResource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = i - 1,
        .baseArrayLayer = 0,
        .layerCount = 1};

    vk::ImageSubresourceLayers blitDstSubResource = {
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .mipLevel = i,
        .baseArrayLayer = 0,
        .layerCount = 1};

    vk::ImageBlit blit = {.srcSubresource = blitSrcSubResource,
                          .srcOffsets = srcOffsets,
                          .dstSubresource = blitDstSubResource,
                          .dstOffsets = dstOffsets};

    commandBuffer.blitImage(m_Image, vk::ImageLayout::eTransferSrcOptimal,
                            m_Image, vk::ImageLayout::eTransferDstOptimal,
                            {blit}, vk::Filter::eLinear);

    TransitionMipLayout(vk::ImageLayout::eTransferSrcOptimal,
                        vk::ImageLayout::eShaderReadOnlyOptimal, i - 1,
                        commandBuffer);

    if (mipWidth > 1) {
      mipWidth /= 2;
    }

    if (mipHeight > 1) {
      mipHeight /= 2;
    }
  }

  TransitionMipLayout(vk::ImageLayout::eTransferDstOptimal,
                      vk::ImageLayout::eShaderReadOnlyOptimal, m_MipLevels - 1,
                      commandBuffer);
  m_Layout = vk::ImageLayout::eShaderReadOnlyOptimal;
}
} // namespace Core
} // namespace SimpleEngine
