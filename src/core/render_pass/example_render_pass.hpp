#pragma once

#include "core/render/render_pass.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
namespace SimpleEngine {
namespace Core {
class ExampleRenderPass : public RenderPass {
public:
  ExampleRenderPass(const std::string &name, const RenderContext &context);
  ~ExampleRenderPass() override;

protected:
  void beginPass(vk::CommandBuffer &commandBuffer) override;
  void render(vk::CommandBuffer &commandBuffer) override;
  void endPass(vk::CommandBuffer &commandBuffer) override;
};
} // namespace Core
} // namespace SimpleEngine
