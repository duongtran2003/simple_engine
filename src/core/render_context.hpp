#pragma once

#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <string>
#include <sys/types.h>
#include <vector>

namespace SimpleEngine {
namespace Core {

class RenderContext {
public:
  struct UniformBufferObject {
    alignas(16) glm::mat4 normalModel;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

    alignas(16) glm::vec3 directionalLightDirection;
    alignas(16) glm::vec3 directionalLightColor;

    alignas(16) glm::vec3 pointLightPosition;
    alignas(16) glm::vec3 pointLightColor;
  };

  struct UboBuffer {
    vk::Buffer buffer{nullptr};
    vk::DeviceMemory memory{nullptr};
    void *mapped = nullptr;
  };

  struct RenderContextCreateInfo {
    std::string appName;
    uint32_t inFlightFrame;
    uint32_t width;
    uint32_t height;
  };

  GLFWwindow *window;
  uint32_t width;
  uint32_t height;

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

  vk::DescriptorPool descriptorPool;
  vk::DescriptorSetLayout descriptorSetLayout;

  vk::DescriptorSetLayout bindlessDescriptorSetLayout;

  std::vector<vk::DescriptorSet> descriptorSets;
  vk::DescriptorSet bindlessDescriptorSets;

  std::vector<UboBuffer> uniformBuffers;

  uint32_t inFlightFrame = 2;
  std::vector<vk::Semaphore> presentCompleteSemaphores;
  std::vector<vk::Semaphore> renderFinishedSemaphores;
  std::vector<vk::Fence> inFlightFences;
  uint32_t frameIndex = 0;

  vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;

  vk::Viewport viewport;
  vk::Rect2D scissor;

  RenderContext() = default;
  RenderContext(const RenderContextCreateInfo &createInfo);

  RenderContext *setMsaaSamples(vk::SampleCountFlagBits sampleCount);
  void *getCurrentFrameUniformBufferPtr();

private:
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

  void createDescriptorPool();
  void createDescriptorSetLayout();
  void createDescriptorSets();
  void createUniformBuffers();

  vk::SampleCountFlagBits getMaxMsaaSampleCount();
};
} // namespace Core
} // namespace SimpleEngine
