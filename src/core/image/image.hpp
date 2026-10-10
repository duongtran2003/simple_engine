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
  struct CreateInfo {
    vk::ImageType Type;
    vk::Format Format;
    vk::Extent3D Extent;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
    vk::SampleCountFlagBits SampleCount = vk::SampleCountFlagBits::e1;
    vk::ImageTiling Tiling = vk::ImageTiling::eOptimal;
    vk::ImageUsageFlags Usage;
    vk::SharingMode SharingMode = vk::SharingMode::eExclusive;
    vk::ImageAspectFlags AspectMask;
    uint32_t Width;
    uint32_t Height;
    uint32_t Channels;
    Enums::Image::ColorSpace ColorSpace = Enums::Image::ColorSpace::eLinear;
  };

  Image() = delete;
  Image(const CreateInfo &createInfo, const RenderContext *context);

  ~Image();
  void Free();
  bool IsValid() const;
  void TransitionLayout(vk::ImageLayout toLayout, bool keepContent,
                        vk::CommandBuffer &commandBuffer);
  void TransitionMipLayout(vk::ImageLayout oldLayout, vk::ImageLayout toLayout,
                           uint32_t mipLevel, vk::CommandBuffer &commandBuffer);

  vk::Image GetImage() const;
  vk::ImageView GetView() const;
  vk::Format GetFormat() const;
  vk::Extent3D GetExtent() const;
  vk::ImageLayout GetLayout() const;

  void UploadData(const void *pixels, vk::DeviceSize size);

private:
  const RenderContext *m_pContext;

  bool m_IsValid = false;

  vk::Image m_Image = nullptr;
  vk::ImageView m_View = nullptr;
  vk::DeviceSize m_Size = 0;
  vk::DeviceMemory m_Memory = nullptr;

  vk::ImageType m_Type;
  vk::Format m_Format;
  vk::Extent3D m_Extent;
  uint32_t m_MipLevels;
  uint32_t m_ArrayLayers;
  vk::SampleCountFlagBits m_SampleCount;
  vk::ImageTiling m_Tiling;
  vk::ImageUsageFlags m_Usage;
  vk::SharingMode m_SharingMode;
  vk::ImageLayout m_Layout;
  vk::ImageAspectFlags m_AspectMask;

  uint32_t m_Width;
  uint32_t m_Height;
  uint32_t m_Channels;

  Enums::Image::ColorSpace m_ColorSpace;

  void Create();
  void CreateImage();
  void AllocMemory();
  void CreateView();

  void GenerateMipmap(vk::CommandBuffer &commandBuffer);
};
} // namespace Core
} // namespace SimpleEngine
