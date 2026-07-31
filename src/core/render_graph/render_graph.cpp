#include "core/render_graph/render_graph.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
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
  passesInsertOrder.push_back(pass->getName());
}

void RenderGraph::removePass(const std::string &passName) {
  auto it = passes.find(passName);

  if (it != passes.end()) {
    passes.erase(it);
  }
  for (auto it = passesInsertOrder.begin(); it != passesInsertOrder.end();
       it++) {
    if (*it == passName) {
      passesInsertOrder.erase(it);
    }
  }
}

void RenderGraph::compile() { sortPasses(); }

void RenderGraph::sortPasses() {
  executionOrder.clear();
  std::queue<std::string> queue;
  std::unordered_map<std::string, bool> visited;
  std::unordered_map<std::string, size_t> incoming;

  std::unordered_map<std::string, std::string> resourceWriters;

  for (const auto &[resourceName, resource] : resources) {
    for (const auto &[passName, pass] : passes) {
      for (const auto &color : pass->getColors()) {
        if (color.resource->getName() == resourceName &&
            (color.accessType == RenderPass::ResourceAccessType::Modify ||
             color.accessType == RenderPass::ResourceAccessType::Write)) {
          resourceWriters[resourceName] = passName;
        }
      }

      const auto &depth = pass->getDepth();
      if (depth.resource != nullptr &&
          depth.resource->getName() == resourceName &&
          (depth.accessType == RenderPass::ResourceAccessType::Modify ||
           depth.accessType == RenderPass::ResourceAccessType::Write)) {
        resourceWriters[resourceName] = passName;
      }
    }
  }

  // List of passes that a specific pass depends on
  std::unordered_map<std::string, std::unordered_set<std::string>> dependencies;

  // List of passes that depend on this pass
  std::unordered_map<std::string, std::unordered_set<std::string>> dependents;

  for (const auto &passName : passesInsertOrder) {
    const auto &pass = passes[passName];
    for (const auto &color : pass->getColors()) {
      if (color.accessType == RenderPass::ResourceAccessType::Modify ||
          color.accessType == RenderPass::ResourceAccessType::Read) {
        auto writerIt = resourceWriters.find(color.resource->getName());
        if (writerIt == resourceWriters.end()) {
          throw std::runtime_error("RenderGraph::sortPasses::ERROR: Pass " +
                                   passName + " reads resource " +
                                   color.resource->getName() +
                                   " that has no writer.");
        }

        std::string writerName = writerIt->second;
        dependencies[passName].insert(writerName);
        dependents[writerName].insert(passName);
      }
    }

    for (const auto &sampled : pass->getSampled()) {
      if (sampled.accessType == RenderPass::ResourceAccessType::Modify ||
          sampled.accessType == RenderPass::ResourceAccessType::Read) {
        auto writerIt = resourceWriters.find(sampled.resource->getName());
        if (writerIt == resourceWriters.end()) {
          throw std::runtime_error("RenderGraph::sortPasses::ERROR: Pass " +
                                   passName + " reads resource " +
                                   sampled.resource->getName() +
                                   " that has no writer.");
        }

        std::string writerName = writerIt->second;
        dependencies[passName].insert(writerName);
        dependents[writerName].insert(passName);
      }
    }

    const auto &depth = pass->getDepth();
    if (depth.accessType == RenderPass::ResourceAccessType::Modify ||
        depth.accessType == RenderPass::ResourceAccessType::Read) {
      auto writerIt = resourceWriters.find(depth.resource->getName());
      if (writerIt == resourceWriters.end()) {
        throw std::runtime_error("RenderGraph::sortPasses::ERROR: Pass " +
                                 passName + " reads resource " +
                                 depth.resource->getName() +
                                 " that has no writer.");
      }

      std::string writerName = writerIt->second;
      dependencies[passName].insert(writerName);
      dependents[writerName].insert(passName);
    }
  }

  for (const auto &[passName, pass] : passes) {
    if (dependencies[passName].empty()) {
      queue.push(passName);
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
