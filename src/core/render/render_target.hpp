#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
class RenderTarget {
private:
  vk::Image colorImage = nullptr;
  vk::DeviceMemory colorMemory = nullptr;
  vk::ImageView colorImageView = nullptr;
  vk::ImageLayout colorLayout = vk::ImageLayout::eUndefined;

  vk::Image depthImage = nullptr;
  vk::DeviceMemory depthMemory = nullptr;
  vk::ImageView depthImageView = nullptr;
  vk::ImageLayout depthLayout = vk::ImageLayout::eUndefined;

  uint32_t width;
  uint32_t height;

  const RenderContext &context;

public:
  RenderTarget(uint32_t w, uint32_t h, const RenderContext &rContext);
  ~RenderTarget();

  vk::ImageView getColorImageView() const;
  vk::ImageView getDepthImageView() const;

  vk::Image getColorImage() const;
  vk::Image getDepthImage() const;

  vk::ImageLayout getColorLayout() const;
  vk::ImageLayout getDepthLayout() const;

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  void transitionColorLayout(vk::CommandBuffer &commandBuffer,
                        vk::ImageLayout dstLayout);

  void transitionDepthLayout(vk::CommandBuffer &commandBuffer,
                        vk::ImageLayout dstLayout);

private:
  void createColorResources();
  void createDepthResources();
};
} // namespace Core
} // namespace SimpleEngine
