#pragma once

#include "core/render_context.hpp"
#include "enums/image_enums.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <sys/types.h>

namespace SimpleEngine {
namespace Core {
class Image {
public:
  struct ImageCreateInfo {
    vk::ImageType type;
    vk::Format format;
    vk::Extent3D extent;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
    vk::ImageUsageFlags usage;
    vk::SharingMode sharingMode = vk::SharingMode::eExclusive;
    vk::ImageAspectFlags aspectMask;
    uint32_t width;
    uint32_t height;
    uint32_t channels;
    Enums::Image::Filter magFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Filter minFilter = Enums::Image::Filter::eLinear;
    Enums::Image::Wrap wrapU = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap wrapV = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::Wrap wrapW = Enums::Image::Wrap::eClampToEdge;
    Enums::Image::ColorSpace colorSpace = Enums::Image::ColorSpace::eLinear;
  };

  Image() = delete;
  Image(const ImageCreateInfo &createInfo, const RenderContext *context);

  ~Image();
  void Create();
  void Free();
  bool IsValid();
  void TransitionLayout(vk::ImageLayout toLayout, bool keepContent,
                        vk::CommandBuffer &commandBuffer);

private:
  const RenderContext *m_pContext;

  bool m_isValid = false;

  vk::Image m_image;
  vk::ImageView m_view;
  vk::DeviceSize m_size;
  vk::DeviceMemory m_memory;
  vk::Sampler m_sampler;

  vk::ImageType m_type;
  vk::Format m_format;
  vk::Extent3D m_extent;
  uint32_t m_mipLevels;
  uint32_t m_arrayLayers;
  vk::SampleCountFlagBits m_sampleCount;
  vk::ImageTiling m_tiling;
  vk::ImageUsageFlags m_usage;
  vk::SharingMode m_sharingMode;
  vk::ImageLayout m_layout;
  vk::ImageAspectFlags m_aspectMask;

  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_channels;

  Enums::Image::Filter m_magFilter;
  Enums::Image::Filter m_minFilter;

  Enums::Image::Wrap m_wrapU;
  Enums::Image::Wrap m_wrapV;
  Enums::Image::Wrap m_wrapW;

  Enums::Image::ColorSpace m_colorSpace;

  void CreateImage();
  void AllocMemory();
  void CreateView();
  void CreateSampler();
};
} // namespace Core
} // namespace SimpleEngine
