#pragma once

#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class MainPass : public RenderPass {
public:
  struct PushConstants {
    alignas(4) uint32_t albedoIndex;
    alignas(4) uint32_t normalIndex;
    alignas(4) uint32_t metallicRoughnessIndex;
    alignas(4) uint32_t uniformIndex;
  };

private:
  vk::PipelineInputAssemblyStateCreateInfo configInputAssembly() override;
  vk::PipelineVertexInputStateCreateInfo configVertexInput() override;
  vk::PipelineLayout createGraphicsPipelineLayout() override;

  vk::VertexInputBindingDescription vertexInputBindingDescription;
  std::vector<vk::VertexInputAttributeDescription>
      vertexInputAttributeDescriptions;

public:
  MainPass(const std::string &name, CreateInfo createInfo,
           const RenderContext &context, ResourceManager &resourceManager);
  ~MainPass() override;
  void init(const CreateInfo &createInfo);
};
} // namespace Core
} // namespace SimpleEngine
