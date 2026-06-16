#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>
namespace SimpleEngine {
namespace Core {
struct RenderContext {
  vk::Device device;
  vk::PhysicalDevice physicalDevice;
  vk::Instance instance;
  vk::Queue graphicsQueue;
  vk::CommandPool commandPool;
  uint32_t graphicsQueueFamilyIndex;
};
} // namespace Core
} // namespace SimpleEngine
