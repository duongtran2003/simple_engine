#include "core/render_graph/render_graph.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderGraph::RenderGraph(const RenderContext &context) : context(context) {}
RenderGraph::~RenderGraph() {
  for (const auto &resource : resources) {
    delete resource.second;
  }
  resources.clear();

  for (const auto &pass : passes) {
    delete pass.second;
  }
  passes.clear();
}

void RenderGraph::setOutputResource(const std::string &resourceName) {
  outputResource = resourceName;
}

GraphResource *RenderGraph::getOutputResource() {
  auto it = resources.find(outputResource);
  if (it == resources.end()) {
    return nullptr;
  }

  return it->second;
}

GraphResource *RenderGraph::getResource(const std::string &resourceName) {
  auto it = resources.find(resourceName);
  if (it == resources.end()) {
    return nullptr;
  }

  return it->second;
}

void RenderGraph::addResource(GraphResource *resource) {
  auto it = resources.find(resource->getName());
  if (it != resources.end()) {
    return;
  }

  resources[resource->getName()] = resource;
}

void RenderGraph::removeResource(const std::string &resourceName) {
  auto it = resources.find(resourceName);

  if (it != resources.end()) {
    resources.erase(it);
  }
}

void RenderGraph::addPass(RenderPass *pass) {
  auto it = passes.find(pass->getName());
  if (it != passes.end()) {
    return;
  }

  passes[pass->getName()] = pass;
  executionOrder.push_back(pass->getName());
}

void RenderGraph::removePass(const std::string &passName) {
  auto it = passes.find(passName);

  if (it != passes.end()) {
    passes.erase(it);
  }

  for (auto it = executionOrder.begin(); it != executionOrder.end(); it++) {
    if (*it == passName) {
      executionOrder.erase(it);
      break;
    }
  }
}

void RenderGraph::compile() { sortPasses(); }

void RenderGraph::sortPasses() {};

void RenderGraph::execute(vk::CommandBuffer &commandBuffer,
                          std::vector<Entity *> &renderObjects) {
  for (size_t i = 0; i < executionOrder.size(); ++i) {
    passes[executionOrder[i]]->execute(commandBuffer, renderObjects);
  }
}
} // namespace Core
} // namespace SimpleEngine
