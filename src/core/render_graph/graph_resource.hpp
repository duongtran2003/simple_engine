#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <sys/types.h>

namespace SimpleEngine {
namespace Core {
class GraphResource {
private:
  const RenderContext &context;

  std::string name;
  vk::Image image = nullptr;
  vk::DeviceMemory memory = nullptr;
  vk::ImageView view = nullptr;
  vk::Format format;
  vk::ImageLayout layout;

  vk::ImageAspectFlags aspectMask;
  vk::ImageUsageFlags usage;

  uint32_t width;
  uint32_t height;

  void createImage();
  void allocateMemory();
  void createView();

  void destroyImage();
  void deallocateMemory();
  void destroyView();

public:
  GraphResource() = delete;
  GraphResource(const std::string &name, uint32_t width, uint32_t height,
                vk::Format format, vk::ImageLayout layout,
                vk::ImageAspectFlags aspectMask, vk::ImageUsageFlags usage,
                const RenderContext &context);
  ~GraphResource();

  const std::string &getName() const;

  vk::Image getImage();
  vk::ImageView getView();
  vk::DeviceMemory getMemory();
  vk::Format getFormat();
  vk::ImageLayout getLayout();
  vk::ImageAspectFlags getAspectMask();

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  void transitionLayout(vk::CommandBuffer &commandBuffer,
                        vk::ImageLayout dstLayout);
};
} // namespace Core
} // namespace SimpleEngine
