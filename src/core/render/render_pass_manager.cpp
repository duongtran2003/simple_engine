#include "core/render/render_pass_manager.hpp"
#include "core/render/render_pass.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <iostream>
#include <queue>
#include <semaphore>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderPassManager::RenderPassManager(const RenderContext &rContext)
    : context(rContext) {}

RenderPass *RenderPassManager::getRenderPass(const std::string &name) {
  auto passIt = passes.find(name);
  if (passIt == passes.end()) {
    return nullptr;
  }

  return passIt->second.get();
}

void RenderPassManager::removeRenderPass(const std::string &name) {
  auto passIt = passes.find(name);

  if (passIt == passes.end()) {
    return;
  }

  passes.erase(passIt);
  isDirty = true;
}

void RenderPassManager::execute(vk::CommandBuffer &commandBuffer) {
  if (isDirty) {
    sortPasses();
  }

  for (const auto &pass : passes) {
    pass.second->execute(commandBuffer);
  }
}

void RenderPassManager::sortPasses() {
  // Topology sort
  sorted.clear();

  std::queue<std::string> queue;
  std::unordered_map<std::string,
                     std::pair<size_t, std::unordered_set<std::string>>>
      depMap;

  // Constructing the depMap:
  for (const auto &[name, passPtr] : passes) {
    if (passPtr->getDependencies().size() == 0) {
      queue.push(passPtr->getName());
    }

    depMap[name].first = passPtr->getDependencies().size();
  }
  if (queue.empty()) {
    throw std::runtime_error("RenderPassManager::sortPasses()::ERROR: Cycle "
                             "detected (queue empty). Aborting.");
  }

  for (const auto &[name, passPtr] : passes) {
    for (const auto &dependency : passPtr->getDependencies()) {
      depMap[dependency].second.insert(name);
    }
  }

  std::unordered_map<std::string, bool> visited;
  std::vector<std::string> sortedNames;

  while (!queue.empty()) {
    auto currentNode = queue.front();
    queue.pop();
    visited[currentNode] = true;
    sortedNames.push_back(currentNode);

    for (const auto &dependent : depMap[currentNode].second) {
      depMap[dependent].first -= 1;
      if (depMap[dependent].first <= 0) {
        queue.push(dependent);
      }
    }
  };

  if (sortedNames.size() != passes.size()) {
    throw std::runtime_error("RenderPassManager::sortPasses()::ERROR: Cycle "
                             "detected (stray passes). Aborting.");
  }

  for (const auto &name : sortedNames) {
    sorted.push_back(passes[name].get());
  }

  isDirty = false;
}
} // namespace Core
} // namespace SimpleEngine
