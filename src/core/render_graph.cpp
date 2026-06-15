#include "core/render_graph.hpp"
#include "vulkan/vulkan.hpp"
#include <functional>
#include <string>
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
} // namespace Core
} // namespace SimpleEngine
