#pragma once

#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph {
private:
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
    std::function<void(vk::CommandBuffer &)> executeFunction;
  };

  std::unordered_map<std::string, Resource> resources;
  std::vector<Pass> passes;
  std::vector<size_t> executionOrder;

  std::vector<vk::Semaphore> semaphores;
  std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs;

  vk::Device &device;

public:
  RenderGraph(vk::Device &device);

  void addResource(const std::string &name, vk::Format format,
                   vk::Extent2D extent, vk::ImageUsageFlags usage,
                   vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);

  void addPass(const std::string &name, const std::vector<std::string> &inputs,
               const std::vector<std::string> &outputs,
               std::function<void(vk::CommandBuffer &)> executeFunction);

  void compile();
};
} // namespace Core
} // namespace SimpleEngine
