#include "core/render_graph/render_pass.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <functional>
#include <string>
#include <unordered_set>

namespace SimpleEngine {
namespace Core {
RenderPass::RenderPass(const std::string &name, const RenderContext &context)
    : context(context) {
  this->name = name;
  active = true;
}

void RenderPass::addInput(const std::string &resourceName) {
  auto resourceIt = inputs.find(resourceName);
  if (resourceIt != inputs.end()) {
    return;
  }

  inputs.insert(resourceName);
}

void RenderPass::addOutput(const std::string &resourceName) {
  auto resourceIt = outputs.find(resourceName);
  if (resourceIt != outputs.end()) {
    return;
  }

  outputs.insert(resourceName);
}

void RenderPass::deleteInput(const std::string &resourceName) {
  auto resourceIt = inputs.find(resourceName);
  if (resourceIt != inputs.end()) {
    inputs.erase(resourceIt);
  }
}

void RenderPass::deleteOutput(const std::string &resourceName) {
  auto resourceIt = outputs.find(resourceName);
  if (resourceIt != outputs.end()) {
    outputs.erase(resourceIt);
  }
}

RenderPass *RenderPass::setIsActive(bool state) {
  active = state;
  return this;
}
RenderPass *RenderPass::setExecuteCallbackFn(
    std::function<void(vk::CommandBuffer &commandBuffer)> fn) {
  executeCallback = fn;
  return this;
}

const std::string &RenderPass::getName() const { return name; }

const std::unordered_set<std::string> &RenderPass::getInputs() const {
  return inputs;
}

const std::unordered_set<std::string> &RenderPass::getOutputs() const {
  return outputs;
}

bool RenderPass::getIsActive() const { return active; }
} // namespace Core
} // namespace SimpleEngine
