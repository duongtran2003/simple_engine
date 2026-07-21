#pragma once

#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class SkyboxPass : public RenderPass {
private:
  vk::PipelineInputAssemblyStateCreateInfo configInputAssembly() override;
  vk::PipelineVertexInputStateCreateInfo configVertexInput() override;
  vk::PipelineLayout createGraphicsPipelineLayout() override;

public:
  SkyboxPass(const std::string &name, CreateInfo createInfo,
             const RenderContext &context, ResourceManager &resourceManager);
  ~SkyboxPass() override;

  void execute(vk::CommandBuffer &commandBuffer,
               std::vector<Entity *> &renderObjects) override;
  void init(const CreateInfo &createInfo);
};
} // namespace Core
} // namespace SimpleEngine
