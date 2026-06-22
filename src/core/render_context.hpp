#pragma once

#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>

namespace SimpleEngine {
namespace Core {

class RenderContext {
public:
  struct RenderContextCreateInfo {
    std::string appName;
    uint32_t inFlightFrame;
    uint32_t width;
    uint32_t height;
  };

  GLFWwindow *window;

  vk::Instance instance;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;

  uint32_t graphicsQueueFamilyIndex;
  vk::Queue graphicsQueue;

  vk::SurfaceKHR surface;
  vk::SwapchainKHR swapChain;
  std::vector<vk::Image> swapChainImages;
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  vk::Extent2D swapChainExtent;
  std::vector<vk::ImageView> swapChainImageViews;

  vk::CommandPool commandPool;
  std::vector<vk::CommandBuffer> commandBuffers;

  vk::Pipeline graphicsPipeline;
  vk::PipelineLayout pipelineLayout;

  uint32_t inFlightFrame = 2;
  std::vector<vk::Semaphore> presentCompleteSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  uint32_t frameIndex = 0;

  vk::Viewport viewport;
  vk::Rect2D scissor;

  RenderContext() = default;
  RenderContext(const RenderContextCreateInfo &createInfo);
  void initWindow(const RenderContextCreateInfo &createInfo);
  void createInstance(const RenderContextCreateInfo &createInfo);
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  void createSwapChainImageViews();
  void createCommandPool();
  void allocateCommandBuffers();
  void createSyncObjects();
  void createViewport();
  void createScissor();
};
} // namespace Core
} // namespace SimpleEngine
