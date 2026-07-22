#include "core/render_graph/render_graph.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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

  // List of passes that a specific pass depends on
  std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;

  // List of passes that depend on this pass
  std::unordered_map<std::string, std::unordered_set<std::string>> dependents;

  for (const auto &[name, pass] : passes) {
    const auto &inputs = pass->getInputs();
    if (inputs.empty()) {
      queue.push(name);
    }
  }

  // TODO: Optimization
  for (const auto &[name, pass] : passes) {
    const auto &inputs = pass->getInputs();
    for (const auto &[otherName, otherPass] : passes) {
      if (name == otherName) {
        continue;
      }

      const auto &otherOutputs = otherPass->getOutputs();
      for (const auto &[inputName, input] : inputs) {
        if (otherOutputs.find(inputName) != otherOutputs.end()) {
          dependencies[name].insert(otherName);
          dependents[otherName].insert(name);
          break;
        }
      }
    }
  }

  if (queue.empty()) {
    throw std::runtime_error("RenderGraph::sortPasses::ERROR: Failed to sort "
                             "passes, cycle detected. Cannot find root pass.");
  }

  while (!queue.empty()) {
    const auto &current = passes[queue.front()];
    queue.pop();
    visited[current->getName()] = true;
    executionOrder.push_back(current->getName());

    for (const auto &dependent : dependents[current->getName()]) {
      dependencies[dependent].erase(current->getName());
      if (dependencies[dependent].size() == 0) {
        queue.push(dependent);
      }
    }
  }

  if (executionOrder.size() != passes.size()) {
    throw std::runtime_error("RenderGraph::sortPasses::ERROR: Failed to sort "
                             "passes, cycle detected");
  }
}

void RenderGraph::execute(vk::CommandBuffer &commandBuffer,
                          std::vector<Entity *> &renderObjects) {
  for (size_t i = 0; i < executionOrder.size(); ++i) {
    passes[executionOrder[i]]->execute(commandBuffer, renderObjects);
  }
}
} // namespace Core
} // namespace SimpleEngine
