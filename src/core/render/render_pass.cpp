#include "core/render/render_pass.h"
#include "core/render/render_target.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
RenderPass::RenderPass(const std::string &name) { this->name = name; }

const std::vector<std::string> &RenderPass::getDependencies() const {
  return dependencies;
}
const std::string &RenderPass::getName() const { return name; }
RenderTarget *RenderPass::getRenderTarget() const { return target; }
bool RenderPass::isEnabled() const { return enabled; }

void RenderPass::execute(vk::CommandBuffer &commandBuffer) {
  if (!enabled) {
    return;
  }

  beginPass(commandBuffer);
  render(commandBuffer);
  endPass(commandBuffer);
}

RenderPass *RenderPass::addDependency(const std::string &dependency) {
  dependencies.push_back(dependency);
  return this;
}

RenderPass *RenderPass::setRenderTarget(RenderTarget *target) {
  this->target = target;
  return this;
}

RenderPass *RenderPass::setEnabled(bool enabled) {
  this->enabled = enabled;
  return this;
}
} // namespace Core
} // namespace SimpleEngine
