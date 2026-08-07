#pragma once

#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class TonemappingPass : public RenderPass {
private:
  vk::PipelineInputAssemblyStateCreateInfo configInputAssembly() override;
  vk::PipelineVertexInputStateCreateInfo configVertexInput() override;
  vk::PipelineLayout createGraphicsPipelineLayout() override;

public:
  enum class SampleSlot { InputImage = 0, TOTAL = 1 };
  struct PushConstants {
    alignas(4) uint32_t inputIndex;
  };

  TonemappingPass(const std::string &name, CreateInfo createInfo,
                  const RenderContext &context,
                  ResourceManager &resourceManager);
  ~TonemappingPass() override;

  void execute(vk::CommandBuffer &commandBuffer,
               std::vector<Entity *> &renderObjects) override;
};
} // namespace Core
} // namespace SimpleEngine
