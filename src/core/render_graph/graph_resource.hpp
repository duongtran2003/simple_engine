#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

namespace SimpleEngine {
namespace Core {
class GraphResource {
private:
  const RenderContext &context;

  uint32_t inFlightFrame;

  std::string name;
  std::vector<vk::Image> images = {};
  std::vector<vk::DeviceMemory> memories = {};
  std::vector<vk::ImageView> views = {};
  vk::Format format;
  std::vector<vk::ImageLayout> layouts = {};
  vk::Sampler sampler;

  vk::ImageAspectFlags aspectMask;
  vk::ImageUsageFlags usage;
  vk::SampleCountFlagBits sampleCount;

  uint32_t width;
  uint32_t height;

  void initiateLayouts(vk::ImageLayout layout);

  void createImages();
  void allocateMemories();
  void createViews();
  void createSampler();

  void destroySampler();
  void destroyImages();
  void deallocateMemories();
  void destroyViews();

public:
  GraphResource() = delete;
  GraphResource(const std::string &name, uint32_t width, uint32_t height,
                vk::Format format, vk::ImageLayout layout,
                vk::ImageAspectFlags aspectMask, vk::ImageUsageFlags usage,
                vk::SampleCountFlagBits sampleCount, uint32_t inFlightFrame,
                const RenderContext &context);
  ~GraphResource();

  const std::string &getName() const;

  vk::Image getImage(uint32_t frameIndex);
  vk::ImageView getView(uint32_t frameIndex);
  vk::DeviceMemory getMemory(uint32_t frameIndex);
  vk::Format getFormat();
  vk::ImageLayout getLayout(uint32_t frameIndex);
  vk::ImageAspectFlags getAspectMask();
  vk::Sampler getSampler();

  uint32_t getWidth() const;
  uint32_t getHeight() const;

  void transitionLayout(vk::CommandBuffer &commandBuffer, uint32_t frameIndex,
                        vk::ImageLayout dstLayout, bool fromLastLayout);
};
} // namespace Core
} // namespace SimpleEngine
