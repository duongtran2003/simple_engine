#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <functional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph;

class RenderPass {
public:
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
  };

private:
  const RenderContext &context;
  ResourceManager &resourceManager;

  std::string name;
  bool active;
  std::unordered_set<std::string> inputs;
  std::unordered_set<std::string> outputs;

  std::function<void(vk::CommandBuffer &commandBuffer)> executeCallback;

  vk::Pipeline graphicsPipeline;
  void createGraphicsPipeline(const CreateInfo &createInfo);

  vk::PipelineRasterizationStateCreateInfo
  configRasterizer(const CreateInfoRasterizer &rasterizerConfig);

  vk::PipelineViewportStateCreateInfo configViewportState();

  vk::PipelineMultisampleStateCreateInfo configMultisampling();

  vk::PipelineDepthStencilStateCreateInfo
  configDepthStencil(const CreateInfoDepthStencil &depthStencilConfig);

  virtual vk::PipelineInputAssemblyStateCreateInfo configInputAssembly() = 0;
  virtual vk::PipelineVertexInputStateCreateInfo configVertexInput() = 0;
  virtual vk::PipelineLayout createGraphicsPipelineLayout();

  std::pair<vk::PipelineShaderStageCreateInfo,
            vk::PipelineShaderStageCreateInfo>
  configShaders(const CreateInfoShader &shaders);

public:
  RenderPass() = delete;
  RenderPass(const std::string &name, CreateInfo createInfo,
             const RenderContext &context, ResourceManager &resourceManager);

  virtual ~RenderPass();

  void addInput(const std::string &resourceName);
  void addOutput(const std::string &resourceName);

  void deleteInput(const std::string &resourceName);
  void deleteOutput(const std::string &resourceName);

  RenderPass *setIsActive(bool state);
  RenderPass *setExecuteCallbackFn(
      std::function<void(vk::CommandBuffer &commandBuffer)> fn);

  const std::string &getName() const;

  const std::unordered_set<std::string> &getInputs() const;
  const std::unordered_set<std::string> &getOutputs() const;

  bool getIsActive() const;
  const vk::Pipeline &getGraphicsPipeline() const;

  void execute(vk::CommandBuffer &commandBuffer);
};
} // namespace Core
} // namespace SimpleEngine
