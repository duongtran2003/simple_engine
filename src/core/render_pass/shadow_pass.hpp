#pragma once

#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float4.hpp>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class ShadowPass : public RenderPass {
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

  uint32_t shadowMapResolution = 1024;

public:
  ShadowPass(const std::string &name, CreateInfo createInfo,
             const RenderContext &context, ResourceManager &resourceManager);
  ~ShadowPass() override;

  ShadowPass *setShadowMapResolution(uint32_t resolution);
  uint32_t getShadowMapResolution() const;

  void execute(vk::CommandBuffer &commandBuffer,
               std::vector<Entity *> &renderObjects) override;
};
} // namespace Core
} // namespace SimpleEngine
