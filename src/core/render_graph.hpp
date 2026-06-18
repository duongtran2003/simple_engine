#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph {
public:
  struct Resource {
    std::string name;
    vk::Format format;
    vk::Extent2D extent;
    vk::ImageUsageFlags usage;
    vk::ImageLayout initialLayout;
    vk::ImageLayout finalLayout;

    vk::Image image = nullptr;
    vk::DeviceMemory memory = nullptr;
    vk::ImageView view = nullptr;
  };

  struct Pass {
    std::string name;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::function<void(vk::CommandBuffer &, RenderGraph &)> executeFunction;
  };

private:
  std::unordered_map<std::string, Resource> resources;
  std::vector<Pass> passes;
  std::vector<size_t> executionOrder;
  std::vector<std::vector<vk::CommandBuffer>> frameCommandBuffers;

  std::vector<vk::Semaphore> semaphores;
  std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs;

  const RenderContext &renderContext;

  void resolveDependencies(std::vector<std::vector<size_t>> &dependencies,
                           std::vector<std::vector<size_t>> &dependents);

  void resolveExecutionOrder(std::vector<std::vector<size_t>> &dependencies,
                             std::vector<std::vector<size_t>> &dependents);

  void createSemaphores(std::vector<vk::Semaphore> &waits,
                        std::vector<vk::Semaphore> &signals,
                        std::vector<vk::PipelineStageFlags> &waitStages,
                        size_t passIndex);

  void createSyncObjects(std::vector<std::vector<size_t>> &dependencies);

  void allocateResources();
  void allocateCommandBuffers();

  uint32_t findMemoryType(uint32_t memoryTypeBits, vk::MemoryPropertyFlags);

  void transitionImageLayout(vk::CommandBuffer &commandBuffer, vk::Image image,
                             vk::ImageLayout srcLayout,
                             vk::ImageLayout dstLayout);

public:
  RenderGraph(const RenderContext &renderContext);

  void addResource(const std::string &name, vk::Format format,
                   vk::Extent2D extent, vk::ImageUsageFlags usage,
                   vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);

  void addPass(
      const std::string &name, const std::vector<std::string> &inputs,
      const std::vector<std::string> &outputs,
      std::function<void(vk::CommandBuffer &, RenderGraph &)> executeFunction);

  void compile();
  void execute(uint32_t frameIndex);

  Resource *getResource(const std::string &name);
};
} // namespace Core
} // namespace SimpleEngine
