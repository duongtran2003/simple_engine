#include "core/render_graph.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderGraph::RenderGraph(vk::Device &device) : device(device) {};

void RenderGraph::addResource(const std::string &name, vk::Format format,
                              vk::Extent2D extent, vk::ImageUsageFlags usage,
                              vk::ImageLayout initialLayout,
                              vk::ImageLayout finalLayout) {
  Resource resource{.name = name,
                    .format = format,
                    .extent = extent,
                    .usage = usage,
                    .initialLayout = initialLayout,
                    .finalLayout = finalLayout};

  resources[name] = resource;
}

void RenderGraph::addPass(
    const std::string &name, const std::vector<std::string> &inputs,
    const std::vector<std::string> &outputs,
    std::function<void(vk::CommandBuffer &)> executeFunction) {
  Pass pass{.name = name,
            .inputs = inputs,
            .outputs = outputs,
            .executeFunction = executeFunction};

  passes.push_back(pass);
}

void RenderGraph::compile() {
  std::vector<std::vector<size_t>>
      dependencies; // For each member, it stores the indices of the passes that
                    // it depends on
  std::vector<std::vector<size_t>>
      dependents; // For each member, it stores the indices of the passes that
                  // depends on it
  std::pmr::unordered_map<std::string, size_t>
      resourceWriters; // The key represents resource name, value is the index
                       // of the pass that output the specific resource

  for (size_t i = 0; i < passes.size(); i++) {
    const auto &pass = passes[i];
    for (const auto &input : pass.inputs) {
      auto it = resourceWriters.find(input);
      if (it != resourceWriters.end()) {
        dependencies[i].push_back(it->second);
        dependents[it->second].push_back(i);
      }
    }

    for (const auto &output : pass.outputs) {
      resourceWriters[output] = i;
    }
  }
}
} // namespace Core
} // namespace SimpleEngine
