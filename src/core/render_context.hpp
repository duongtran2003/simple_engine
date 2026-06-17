#pragma once

#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>

namespace SimpleEngine {
namespace Core {
struct RenderContext {
  GLFWwindow *window;

  vk::Instance instance;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;

  uint32_t queueIndex = 0;
  vk::Queue graphicsQueue;

  vk::SurfaceKHR surface;
  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::ImageView> swapChainImageViews;

  vk::CommandPool commandPool;
  uint32_t graphicsQueueFamilyIndex;

  std::vector<vk::Semaphore> presentCompleteSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  uint32_t frameIndex = 0;
};
} // namespace Core
} // namespace SimpleEngine
