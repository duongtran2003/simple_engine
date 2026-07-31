#include "core/render_graph/tonemapping_pass.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace SimpleEngine {
namespace Core {
TonemappingPass::TonemappingPass(const std::string &name, CreateInfo createInfo,
                                 const RenderContext &context,
                                 ResourceManager &resourceManager)
    : RenderPass(name, context, resourceManager) {
  sampledResources.resize(static_cast<size_t>(SampleSlot::TOTAL));
  CreateInfo passCreateInfo{
      .shaders = {.vertShader = "tonemapping_pass.vert",
                  .fragShader = "tonemapping_pass.frag"},
      .rasterizer = {.enableRasterizerDiscard = vk::False,
                     .polygonMode = vk::PolygonMode::eFill,
                     .cullMode = vk::CullModeFlagBits::eNone,
                     .frontFace = vk::FrontFace::eCounterClockwise},
      .colorBlending = {.enableColorBlending = vk::False,
                        .colorBlendingWriteMask =
                            vk::ColorComponentFlagBits::eR |
                            vk::ColorComponentFlagBits::eG |
                            vk::ColorComponentFlagBits::eB |
                            vk::ColorComponentFlagBits::eA,
                        .enableColorBlendingLogicOp = vk::False,
                        .colorBlendingLogicOp = vk::LogicOp::eCopy},
      .depthStencil = {.enableDepthTest = vk::False,
                       .enableDepthWrite = vk::False,
                       .depthCompareOp = vk::CompareOp::eLess,
                       .enableDepthBoundsTest = vk::False,
                       .enableStencilTest = vk::False},
      .rendering = createInfo.rendering,
      .renderArea = {.offset = {0.0f, 0.0f},
                     .extent = {context.swapChainExtent.width,
                                context.swapChainExtent.height}}};
  init(passCreateInfo);
}

TonemappingPass::~TonemappingPass() {
  // TODO: Proper destructor
}

vk::PipelineInputAssemblyStateCreateInfo
TonemappingPass::configInputAssembly() {
  return {.topology = vk::PrimitiveTopology::eTriangleList};
}

vk::PipelineVertexInputStateCreateInfo TonemappingPass::configVertexInput() {
  return {.vertexBindingDescriptionCount = 0,
          .pVertexBindingDescriptions = nullptr,
          .vertexAttributeDescriptionCount = 0,
          .pVertexAttributeDescriptions = nullptr};
}

vk::PipelineLayout TonemappingPass::createGraphicsPipelineLayout() {
  auto layouts = context.getGlobalDescriptorSetLayouts();

  vk::PushConstantRange pushConstantRange{
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(PushConstants)};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}

void TonemappingPass::execute(vk::CommandBuffer &commandBuffer,
                              std::vector<Entity *> &renderObjects) {
  std::vector<vk::RenderingAttachmentInfoKHR> renderColorAttachments;
  prepareRenderColorAttachments(renderColorAttachments, commandBuffer);
  auto renderDepthAttachment = prepareRenderDepthAttachment(commandBuffer);
  prepareRenderSampledResources(commandBuffer);

  vk::RenderingInfoKHR renderingInfo;
  prepareRenderingInfo(renderingInfo, renderColorAttachments,
                       renderDepthAttachment);

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             getGraphicsPipeline());
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);

  auto descriptorSets = context.getGlobalDescriptorSets();

  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0,
      descriptorSets.size(), descriptorSets.data(), 0, nullptr);

  PushConstants pushConstant{
      .inputIndex =
          sampledResources[static_cast<size_t>(SampleSlot::InputImage)]
              .resource->getBindSlot()};
  commandBuffer.pushConstants(graphicsPipelineLayout,
                              vk::ShaderStageFlagBits::eFragment, 0,
                              sizeof(PushConstants), &pushConstant);

  commandBuffer.draw(6, 1, 0, 0);
  commandBuffer.endRendering();
}
} // namespace Core
} // namespace SimpleEngine
