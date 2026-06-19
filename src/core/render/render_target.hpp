#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderTarget {
private:
  std::vector<vk::Image> colorImages;
  std::vector<vk::DeviceMemory> colorMemories;
  std::vector<vk::ImageView> colorImageViews;
  std::vector<vk::ImageLayout> colorLayouts;
  std::vector<vk::Format> colorFormats;

  vk::Image depthImage = nullptr;
  vk::DeviceMemory depthMemory = nullptr;
  vk::ImageView depthImageView = nullptr;
  vk::ImageLayout depthLayout = vk::ImageLayout::eUndefined;

  uint32_t width;
  uint32_t height;

  const RenderContext &context;

public:
  RenderTarget(uint32_t w, uint32_t h, std::vector<vk::Format> &colorFormats,
               const RenderContext &rContext);
  ~RenderTarget();

  const std::vector<vk::ImageView> &getColorImageViews() const;
  vk::ImageView getColorImageView(size_t index) const;
  vk::ImageView getDepthImageView() const;

  const std::vector<vk::Image> &getColorImages() const;
  vk::Image getColorImage(size_t index) const;
  vk::Image getDepthImage() const;

  const std::vector<vk::ImageLayout> &getColorLayouts() const;
  vk::ImageLayout getColorLayout(size_t index) const;
  vk::ImageLayout getDepthLayout() const;

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  void transitionColorLayout(vk::CommandBuffer &commandBuffer, size_t index,
                             vk::ImageLayout dstLayout);

  void transitionDepthLayout(vk::CommandBuffer &commandBuffer,
                             vk::ImageLayout dstLayout);

private:
  void createColorResources();
  void createDepthResources();
};
} // namespace Core
} // namespace SimpleEngine
