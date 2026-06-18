#pragma once

#include "core/render/render_target.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>
namespace SimpleEngine {
namespace Core {
class RenderPass {
private:
  std::string name;
  std::vector<std::string> dependencies;
  RenderTarget *target = nullptr;
  bool enabled = true;

public:
  RenderPass(const std::string &name);
  virtual ~RenderPass() = default;

  const std::vector<std::string> &getDependencies() const;
  const std::string &getName() const;
  RenderTarget *getRenderTarget() const;
  bool isEnabled() const;

  RenderPass *addDependency(const std::string &dependency);
  RenderPass *setRenderTarget(RenderTarget *target);
  RenderPass *setEnabled(bool enabled);

  virtual void execute(vk::CommandBuffer &commandBuffer);

protected:
  virtual void beginPass(vk::CommandBuffer &commandBuffer);
  virtual void render(vk::CommandBuffer &commandBuffer);
  virtual void endPass(vk::CommandBuffer &commandBuffer);
};
} // namespace Core
} // namespace SimpleEngine
