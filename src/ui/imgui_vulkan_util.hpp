#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace UI {
class ImGuiVulkanUtil {
private:
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
};
} // namespace UI
} // namespace SimpleEngine
