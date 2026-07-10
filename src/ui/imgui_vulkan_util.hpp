#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace UI {
class ImGuiVulkanUtil {
private:
  const Core::RenderContext &context;

  vk::Sampler sampler;

  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  void *vertexBufferMapped;

  vk::Buffer indexBuffer;
  vk::DeviceMemory indexBufferMemory;
  void *indexBufferMapped;

  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;

  vk::Image fontImage;
  vk::DeviceMemory fontImageMemory;
  vk::ImageView fontImageView;

  vk::PipelineCache pipelineCache;
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline pipeline;

  vk::DescriptorPool descriptorPool;
  vk::DescriptorSetLayout descriptorSetLayout;
  vk::DescriptorSet descriptorSet;
};
} // namespace UI
} // namespace SimpleEngine
