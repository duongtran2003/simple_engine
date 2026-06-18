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

  vk::Image depthImage = nullptr;
  vk::DeviceMemory depthMemory = nullptr;
  vk::ImageView depthImageView = nullptr;

  uint32_t width;
  uint32_t height;

  const RenderContext &context;

public:
  RenderTarget(uint32_t w, uint32_t h, const RenderContext &rContext);
  ~RenderTarget();

  vk::ImageView getColorImageView() const;
  vk::ImageView getDepthImageView() const;

  uint32_t getWidth() const;
  uint32_t getHeight() const;

private:
  void createColorResources();
  void createDepthResources();
};
} // namespace Core
} // namespace SimpleEngine
