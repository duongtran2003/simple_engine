#pragma once

#include "helpers/vulkan_helper.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {

class RenderContext {
public:
  struct UniformBufferObject {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 cameraPos;

    alignas(4) float directionalLightIntensity;
    alignas(16) glm::vec3 directionalLightDirection;
    alignas(16) glm::vec3 directionalLightColor;

    alignas(4) float pointLightIntensity;
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
  vk::PhysicalDeviceProperties physicalDeviceProperty;

  vk::DebugUtilsMessengerEXT debugMessenger = nullptr;

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

  vk::DescriptorPool descriptorPool;

  vk::DescriptorSetLayout descriptorSetLayout;
  std::vector<vk::DescriptorSet> descriptorSets;

  vk::DescriptorSetLayout bindlessDescriptorSetLayout;
  vk::DescriptorSet bindlessDescriptorSets;

  vk::DescriptorSetLayout ssboDescriptorSetLayout;
  std::vector<vk::DescriptorSet> ssboDescriptorSets;

  vk::DescriptorSetLayout bindlessResourceDescriptorSetLayout;
  std::vector<vk::DescriptorSet> bindlessResourceDescriptorSets;

  std::vector<UboBuffer> uniformBuffers;
  std::vector<UboBuffer> storageBuffers;

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

  void updateDeltaTime();
  float getDeltaTime() const;

  void recreateSwapChain();

  RenderContext *setMsaaSamples(vk::SampleCountFlagBits sampleCount);
  void *getCurrentFrameUniformBufferPtr();
  void *getCurrentFrameStorageBufferPtr() const;
  std::vector<vk::DescriptorSetLayout> getGlobalDescriptorSetLayouts() const;
  std::vector<vk::DescriptorSet> getGlobalDescriptorSets() const;

  template <typename T> void createStorageBuffers(size_t numObjects);

  bool didFrameBufferSizeChange() const;

private:
  double lastFrametime;
  float deltaTime = 0.0f;

  bool framebufferSizeChanged = false;

  void initWindow(const RenderContextCreateInfo &createInfo);
  void createInstance(const RenderContextCreateInfo &createInfo);
  void configureDebugMessages();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  void createSwapChainImageViews();

  void cleanupSwapChain();

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

template <typename T>
void RenderContext::createStorageBuffers(size_t numObjects) {
  size_t allocateCount = std::max(numObjects, static_cast<size_t>(1));
  vk::DeviceSize bufferSize = sizeof(T) * allocateCount;

  for (size_t i = 0; i < inFlightFrame; i++) {
    const auto &[buffer, memory] = Helper::VulkanHelper::createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent,
        *this);

    UboBuffer uboBuffer{
        .buffer = std::move(buffer),
        .memory = std::move(memory),
    };
    uboBuffer.mapped = device.mapMemory(uboBuffer.memory, 0, bufferSize);
    storageBuffers.push_back(uboBuffer);

    vk::DescriptorBufferInfo bufferInfo{
        .buffer = storageBuffers[i].buffer, .offset = 0, .range = bufferSize};

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = ssboDescriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eStorageBuffer,
        .pBufferInfo = &bufferInfo};

    device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
  }
}
} // namespace Core
} // namespace SimpleEngine
