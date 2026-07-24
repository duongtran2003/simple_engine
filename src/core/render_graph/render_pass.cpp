#include "core/render_graph/render_pass.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
class RenderGraph;

RenderPass::RenderPass(const std::string &name, const RenderContext &context,
                       ResourceManager &resourceManager)
    : resourceManager(resourceManager), context(context) {
  this->name = name;
  active = true;
}

RenderPass::~RenderPass() {
  // TODO
};

void RenderPass::createGraphicsPipeline(const CreateInfo &createInfo) {

  auto [vertexShaderStageInfo, fragmentShaderStageInfo] =
      configShaders(createInfo.shaders);
  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,
                                                      fragmentShaderStageInfo};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo = configVertexInput();

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly =
      configInputAssembly();

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicState = {
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  vk::PipelineViewportStateCreateInfo viewportState = configViewportState();

  vk::PipelineRasterizationStateCreateInfo rasterizer =
      configRasterizer(createInfo.rasterizer);

  vk::PipelineMultisampleStateCreateInfo multisampling = configMultisampling();

  vk::PipelineDepthStencilStateCreateInfo depthStencil =
      configDepthStencil(createInfo.depthStencil);

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = createInfo.colorBlending.enableColorBlending,
      .colorWriteMask = createInfo.colorBlending.colorBlendingWriteMask};
  vk::PipelineColorBlendStateCreateInfo colorBlending = {
      .logicOpEnable = createInfo.colorBlending.enableColorBlendingLogicOp,
      .logicOp = createInfo.colorBlending.colorBlendingLogicOp,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  graphicsPipelineLayout = createGraphicsPipelineLayout();
  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo = {
      .colorAttachmentCount =
          static_cast<uint32_t>(createInfo.rendering.colorFormats.size()),
      .pColorAttachmentFormats = createInfo.rendering.colorFormats.data(),
      .depthAttachmentFormat = createInfo.rendering.depthFormat};

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
      .pNext = &pipelineRenderingCreateInfo,
      .stageCount = 2,
      .pStages = shaderStages,
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pDepthStencilState = &depthStencil,
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = graphicsPipelineLayout,
      .renderPass = nullptr};

  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      pipelineCreateInfoChain = {graphicsPipelineCreateInfo,
                                 pipelineRenderingCreateInfo};

  auto [result, pipelines] = context.device.createGraphicsPipelines(
      nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

  if (result == vk::Result::eSuccess) {
    vk::Pipeline pipeline = pipelines[0];
    graphicsPipeline = pipeline;
  }
}

std::pair<vk::PipelineShaderStageCreateInfo, vk::PipelineShaderStageCreateInfo>
RenderPass::configShaders(const CreateInfoShader &shaders) {
  std::filesystem::path vertexShaderPath =
      std::filesystem::path("shaders") / (shaders.vertShader + ".spv");
  std::string vertexShaderName = this->name + "_" + shaders.vertShader;

  std::filesystem::path fragmentShaderPath =
      std::filesystem::path("shaders") / (shaders.fragShader + ".spv");
  std::string fragmentShaderName = this->name + "_" + shaders.fragShader;

  vertexShaderHandle = resourceManager.load<Shader>(
      vertexShaderName, vk::ShaderStageFlagBits::eVertex,
      vertexShaderPath.string());
  fragmentShaderHandle = resourceManager.load<Shader>(
      fragmentShaderName, vk::ShaderStageFlagBits::eFragment,
      fragmentShaderPath.string());

  vk::PipelineShaderStageCreateInfo vertexShaderStageInfo = {
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = vertexShaderHandle->getShaderModule(),
      .pName = "vertMain"};

  vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo = {
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = fragmentShaderHandle->getShaderModule(),
      .pName = "fragMain"};

  return {std::move(vertexShaderStageInfo), std::move(fragmentShaderStageInfo)};
}

vk::PipelineRasterizationStateCreateInfo
RenderPass::configRasterizer(const CreateInfoRasterizer &rasterizerConfig) {
  return {.rasterizerDiscardEnable = rasterizerConfig.enableRasterizerDiscard,
          .polygonMode = rasterizerConfig.polygonMode,
          .cullMode = rasterizerConfig.cullMode,
          .frontFace = rasterizerConfig.frontFace,
          .depthBiasEnable = vk::False,
          .depthBiasClamp = vk::False,
          .lineWidth = 1.0f};
}

vk::PipelineViewportStateCreateInfo RenderPass::configViewportState() {
  return {.viewportCount = 1,
          .pViewports = &context.viewport,
          .scissorCount = 1,
          .pScissors = &context.scissor};
}

vk::PipelineMultisampleStateCreateInfo RenderPass::configMultisampling() {
  return {.rasterizationSamples = context.msaaSamples,
          .sampleShadingEnable = vk::True,
          .minSampleShading = .2f};
}

vk::PipelineDepthStencilStateCreateInfo RenderPass::configDepthStencil(
    const CreateInfoDepthStencil &depthStencilConfig) {
  return {.depthTestEnable = depthStencilConfig.enableDepthTest,
          .depthWriteEnable = depthStencilConfig.enableDepthWrite,
          .depthCompareOp = depthStencilConfig.depthCompareOp,
          .depthBoundsTestEnable = depthStencilConfig.enableDepthBoundsTest,
          .stencilTestEnable = depthStencilConfig.enableStencilTest};
}

void RenderPass::addInput(GraphResource *resource) {
  auto resourceIt = inputs.find(resource->getName());
  if (resourceIt != inputs.end()) {
    return;
  }

  inputs[resource->getName()] = resource;
}

void RenderPass::addOutput(GraphResource *resource) {
  auto resourceIt = outputs.find(resource->getName());
  if (resourceIt != outputs.end()) {
    return;
  }

  outputs[resource->getName()] = resource;
}

void RenderPass::deleteInput(const std::string &resourceName) {
  auto resourceIt = inputs.find(resourceName);
  if (resourceIt != inputs.end()) {
    inputs.erase(resourceIt);
  }
}

void RenderPass::deleteOutput(const std::string &resourceName) {
  auto resourceIt = outputs.find(resourceName);
  if (resourceIt != outputs.end()) {
    outputs.erase(resourceIt);
  }
}

RenderPass *RenderPass::setIsActive(bool state) {
  active = state;
  return this;
}
RenderPass *RenderPass::setExecuteCallbackFn(
    std::function<void(vk::CommandBuffer &commandBuffer)> fn) {
  executeCallback = fn;
  return this;
}

const std::string &RenderPass::getName() const { return name; }

const std::unordered_map<std::string, GraphResource *> &
RenderPass::getInputs() const {
  return inputs;
}

const std::unordered_map<std::string, GraphResource *> &
RenderPass::getOutputs() const {
  return outputs;
}

bool RenderPass::getIsActive() const { return active; }
const vk::Pipeline &RenderPass::getGraphicsPipeline() const {
  return graphicsPipeline;
}

RenderPass *RenderPass::setColorAttachment(GraphResource *color) {
  colorAttachment = color;
  return this;
}

RenderPass *RenderPass::setDepthAttachment(GraphResource *depth) {
  depthAttachment = depth;
  return this;
}
} // namespace Core
} // namespace SimpleEngine
