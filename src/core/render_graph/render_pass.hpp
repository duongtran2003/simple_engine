#pragma once

#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#include "vulkan/vulkan.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <glm/ext/vector_float2.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph;

class RenderPass {
public:
  enum class ResourceAccessType { Write, Modify, Read };
  struct ResourceUsage {
    GraphResource *resource;
    ResourceAccessType accessType;
  };

  struct RenderArea {
    glm::vec2 offset;
    glm::vec2 extent;
  };

  struct CreateInfoShader {
    std::string vertShader;
    std::string fragShader;
  };

  struct CreateInfoRasterizer {
    bool enableRasterizerDiscard;
    vk::PolygonMode polygonMode;
    vk::CullModeFlagBits cullMode;
    vk::FrontFace frontFace;
  };

  struct CreateInfoColorBlending {
    bool enableColorBlending;
    vk::ColorComponentFlags colorBlendingWriteMask;
    bool enableColorBlendingLogicOp;
    vk::LogicOp colorBlendingLogicOp;
  };

  struct CreateInfoDepthStencil {
    bool enableDepthTest;
    bool enableDepthWrite;
    vk::CompareOp depthCompareOp;
    bool enableDepthBoundsTest;
    bool enableStencilTest;
  };

  struct CreateInfoRendering {
    std::vector<vk::Format> colorFormats;
    vk::Format depthFormat;
  };

  struct CreateInfo {
    CreateInfoShader shaders;
    CreateInfoRasterizer rasterizer;
    CreateInfoColorBlending colorBlending;
    CreateInfoDepthStencil depthStencil;
    CreateInfoRendering rendering;
    RenderArea renderArea;
  };

private:
  ResourceManager &resourceManager;

  std::string name;
  bool active;

  RenderArea renderArea;

  std::function<void(vk::CommandBuffer &commandBuffer)> executeCallback;

  vk::Pipeline graphicsPipeline;

  vk::PipelineRasterizationStateCreateInfo
  configRasterizer(const CreateInfoRasterizer &rasterizerConfig);

  vk::PipelineViewportStateCreateInfo configViewportState();

  vk::PipelineMultisampleStateCreateInfo configMultisampling();

  vk::PipelineDepthStencilStateCreateInfo
  configDepthStencil(const CreateInfoDepthStencil &depthStencilConfig);

  virtual vk::PipelineInputAssemblyStateCreateInfo configInputAssembly() = 0;
  virtual vk::PipelineVertexInputStateCreateInfo configVertexInput() = 0;
  virtual vk::PipelineLayout createGraphicsPipelineLayout() = 0;

  ResourceHandle<Shader> vertexShaderHandle;
  ResourceHandle<Shader> fragmentShaderHandle;
  std::pair<vk::PipelineShaderStageCreateInfo,
            vk::PipelineShaderStageCreateInfo>
  configShaders(const CreateInfoShader &shaders);

protected:
  const RenderContext &context;
  void init(const CreateInfo &createInfo);
  void createGraphicsPipeline(const CreateInfo &createInfo);

  vk::PipelineLayout graphicsPipelineLayout;

  std::vector<ResourceUsage> colorAttachments;
  ResourceUsage depthAttachment = {.resource = nullptr};
  std::vector<ResourceUsage> sampledResources;

  vk::VertexInputBindingDescription vertexInputBindingDescription;
  std::vector<vk::VertexInputAttributeDescription>
      vertexInputAttributeDescriptions;

public:
  RenderPass() = delete;
  RenderPass(const std::string &name, const RenderContext &context,
             ResourceManager &resourceManager);

  virtual ~RenderPass();

  RenderPass *setColors(std::vector<ResourceUsage> colors);
  RenderPass *setDepth(ResourceUsage depth);
  template <typename T>
  RenderPass *setSampled(ResourceUsage sampled, T sampleSlot);
  RenderPass *setIsActive(bool state);
  RenderPass *setExecuteCallbackFn(
      std::function<void(vk::CommandBuffer &commandBuffer)> fn);

  const std::string &getName() const;

  bool getIsActive() const;
  const vk::Pipeline &getGraphicsPipeline() const;
  const RenderArea &getRenderArea() const;

  const std::vector<ResourceUsage>& getColors() const;
  const std::vector<ResourceUsage>& getSampled() const;
  const ResourceUsage& getDepth() const;

  virtual void execute(vk::CommandBuffer &commandBuffer,
                       std::vector<Entity *> &renderObjects) = 0;

  void prepareRenderColorAttachments(
      std::vector<vk::RenderingAttachmentInfoKHR> &colors,
      vk::CommandBuffer &commandBuffer);

  std::optional<vk::RenderingAttachmentInfoKHR>
  prepareRenderDepthAttachment(vk::CommandBuffer &commandBuffer);
  void prepareRenderSampledResources(vk::CommandBuffer &commandBuffer);

  void prepareRenderingInfo(
      vk::RenderingInfoKHR &renderingInfo,
      const std::vector<vk::RenderingAttachmentInfoKHR> &colors,
      const std::optional<vk::RenderingAttachmentInfoKHR> &depth);
};

template <typename T>
RenderPass *RenderPass::setSampled(ResourceUsage sampled, T sampleSlot) {
  size_t slot = static_cast<size_t>(sampleSlot);
  assert(sampledResources.size() > slot);
  sampledResources[slot] = std::move(sampled);
  return this;
}

} // namespace Core
} // namespace SimpleEngine
