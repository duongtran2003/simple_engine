#pragma once

#include "core/render/render_target.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>
namespace SimpleEngine {
namespace Core {
class RenderPass {
private:
  std::string name;
  std::vector<std::string> dependencies;
  bool enabled = true;

public:
  RenderPass(const std::string &name, const RenderContext &rContext);
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
  RenderTarget *target = nullptr;
  const RenderContext &context;

  virtual void beginPass(vk::CommandBuffer &commandBuffer) = 0;
  virtual void render(vk::CommandBuffer &commandBuffer) = 0;
  virtual void endPass(vk::CommandBuffer &commandBuffer) = 0;
};
} // namespace Core
} // namespace SimpleEngine
