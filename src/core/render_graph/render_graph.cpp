#include "core/render_graph/render_graph.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace SimpleEngine {
namespace Core {
RenderGraph::RenderGraph(const RenderContext &context) : context(context) {}
RenderGraph::~RenderGraph() {
  for (const auto &resource : resources) {
    delete resource.second;
  }

  for (const auto &pass : passes) {
    delete pass.second;
  }
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
}

void RenderGraph::removePass(const std::string &passName) {
  auto it = passes.find(passName);

  if (it != passes.end()) {
    passes.erase(it);
  }
}

void RenderGraph::compile() { sortPasses(); }

void RenderGraph::sortPasses() {
  executionOrder.clear();
  std::queue<std::string> queue;
  std::unordered_map<std::string, bool> visited;
  std::unordered_map<std::string, size_t> incoming;
  std::unordered_map<std::string, std::unordered_set<std::string>> dependents;

  for (const auto &[name, pass] : passes) {
    const auto &dependencies = pass->getInputs();
    if (dependencies.empty()) {
      queue.push(name);
    }

    incoming[name] = pass->getInputs().size();
    dependents[name] = {};
  }

  for (const auto &[name, pass] : passes) {
    for (const auto &dependency : pass->getInputs()) {
      dependents[dependency].insert(name);
    }
  }

  if (queue.empty()) {
    throw std::runtime_error("RenderGraph::sortPasses::ERROR: Failed to sort "
                             "passes, cycle detected.");
  }

  while (!queue.empty()) {
    const auto &current = passes[queue.front()];
    queue.pop();
    visited[current->getName()] = true;
    executionOrder.push_back(current->getName());

    for (const auto &dependent : dependents[current->getName()]) {
      incoming[dependent] -= 1;
      if (incoming[dependent] == 0) {
        queue.push(dependent);
      }
    }
  }

  if (executionOrder.size() != passes.size()) {
    throw std::runtime_error("RenderGraph::sortPasses::ERROR: Failed to sort "
                             "passes, cycle detected");
  }
}

void RenderGraph::execute(vk::CommandBuffer &commandBuffer) {
  for (const auto &pass : executionOrder) {
    const auto &renderPass = passes[pass];
    renderPass->execute(commandBuffer);
  }
}
} // namespace Core
} // namespace SimpleEngine
