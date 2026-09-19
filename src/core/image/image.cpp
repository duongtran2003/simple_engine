#include "core/image/image.hpp"
#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <algorithm>
#include <cstdint>

namespace SimpleEngine {
namespace Core {
Image::Image(vk::ImageType type, vk::Format format, vk::Extent3D extent,
             uint32_t mipLevels, uint32_t arrayLayers,
             vk::SampleCountFlagBits sampleCount, vk::ImageTiling tiling,
             vk::ImageUsageFlags usage, vk::SharingMode sharingMode,
             vk::ImageAspectFlags aspectMask, uint32_t width, uint32_t height,
             uint32_t channels, Enums::Image::Filter magFilter,
             Enums::Image::Filter minFilter, Enums::Image::Wrap wrapU,
             Enums::Image::Wrap wrapV, Enums::Image::Wrap wrapW,
             Enums::Image::ColorSpace colorSpace,
             const RenderContext *context) {
  m_type = type;
  m_format = format;
  m_extent = extent;
  m_mipLevels = mipLevels;
  m_arrayLayers = arrayLayers;
  m_sampleCount = sampleCount;
  m_tiling = tiling;
  m_usage = usage;
  m_sharingMode = sharingMode;
  m_aspectMask = aspectMask;
  m_pContext = context;

  m_width = width;
  m_height = height;
  m_channels = channels;

  m_magFilter = magFilter;
  m_minFilter = minFilter;

  m_wrapU = wrapU;
  m_wrapV = wrapV;
  m_wrapW = wrapW;

  m_colorSpace = colorSpace;

  Create();
}

Image::~Image() {};

void Image::Create() {
  CreateImage();
  AllocMemory();
  CreateView();
  CreateSampler();
  m_isValid = true;
}

void Image::Free() {
  if (!m_isValid) {
    return;
  }

  m_pContext->device.destroySampler(m_sampler);
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

void Image::CreateSampler() {
  vk::Filter vkMagFilter, vkMinFilter;
  if (m_magFilter == Enums::Image::Filter::eLinear) {
    vkMagFilter = vk::Filter::eLinear;
  } else {
    vkMagFilter = vk::Filter::eNearest;
  }
  if (m_minFilter == Enums::Image::Filter::eLinear) {
    vkMinFilter = vk::Filter::eLinear;
  } else {
    vkMinFilter = vk::Filter::eNearest;
  }

  vk::SamplerAddressMode vkUWrap, vkVWrap, vkWWrap;
  if (m_wrapU == Enums::Image::Wrap::eMirroredRepeat) {
    vkUWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapU == Enums::Image::Wrap::eRepeat) {
    vkUWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkUWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_wrapV == Enums::Image::Wrap::eMirroredRepeat) {
    vkVWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapV == Enums::Image::Wrap::eRepeat) {
    vkVWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkVWrap = vk::SamplerAddressMode::eClampToEdge;
  }
  if (m_wrapW == Enums::Image::Wrap::eMirroredRepeat) {
    vkWWrap = vk::SamplerAddressMode::eMirroredRepeat;
  } else if (m_wrapW == Enums::Image::Wrap::eRepeat) {
    vkWWrap = vk::SamplerAddressMode::eRepeat;
  } else {
    vkWWrap = vk::SamplerAddressMode::eClampToEdge;
  }

  auto anisotropyEnable =
      m_sampleCount != vk::SampleCountFlagBits::e1 ? vk::True : vk::False;

  vk::PhysicalDeviceProperties deviceProperties =
      m_pContext->physicalDevice.getProperties();
  float maxAnisotropy =
      std::min(16.0f, deviceProperties.limits.maxSamplerAnisotropy);

  vk::SamplerCreateInfo info{.magFilter = vkMagFilter,
                             .minFilter = vkMinFilter,
                             .addressModeU = vkUWrap,
                             .addressModeV = vkVWrap,
                             .addressModeW = vkWWrap,
                             .mipLodBias = -0.5f,
                             .anisotropyEnable = anisotropyEnable,
                             .maxAnisotropy = maxAnisotropy,
                             .compareEnable = vk::False,
                             .compareOp = vk::CompareOp::eAlways,
                             .minLod = 0.0f,
                             .maxLod = vk::LodClampNone,
                             .borderColor = vk::BorderColor::eIntOpaqueBlack,
                             .unnormalizedCoordinates = vk::False};

  m_sampler = m_pContext->device.createSampler(info);
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

bool Image::IsValid() { return m_isValid; }
} // namespace Core
} // namespace SimpleEngine
