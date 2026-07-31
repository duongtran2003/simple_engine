#include "core/render_graph/skybox_pass.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace SimpleEngine {
namespace Core {
SkyboxPass::SkyboxPass(const std::string &name, CreateInfo createInfo,
                       const RenderContext &context,
                       ResourceManager &resourceManager)
    : RenderPass(name, context, resourceManager) {
  CreateInfo passCreateInfo{
      .shaders = {.vertShader = "skybox_pass.vert",
                  .fragShader = "skybox_pass.frag"},
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
      .depthStencil = {.enableDepthTest = vk::True,
                       .enableDepthWrite = vk::False,
                       .depthCompareOp = vk::CompareOp::eLessOrEqual,
                       .enableDepthBoundsTest = vk::False,
                       .enableStencilTest = vk::False},
      .rendering = createInfo.rendering,
      .renderArea = {.offset = {0.0f, 0.0f},
                     .extent = {context.swapChainExtent.width,
                                context.swapChainExtent.height}}};

  init(passCreateInfo);
}

SkyboxPass::~SkyboxPass() {
  // TODO
}

vk::PipelineInputAssemblyStateCreateInfo SkyboxPass::configInputAssembly() {
  return {.topology = vk::PrimitiveTopology::eTriangleList};
}

vk::PipelineVertexInputStateCreateInfo SkyboxPass::configVertexInput() {
  return {.vertexBindingDescriptionCount = 0,
          .pVertexBindingDescriptions = nullptr,
          .vertexAttributeDescriptionCount = 0,
          .pVertexAttributeDescriptions = nullptr};
}

vk::PipelineLayout SkyboxPass::createGraphicsPipelineLayout() {
  auto layouts = context.getGlobalDescriptorSetLayouts();

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}

void SkyboxPass::execute(vk::CommandBuffer &commandBuffer,
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

  commandBuffer.draw(36, 1, 0, 0);
  commandBuffer.endRendering();
}
} // namespace Core
} // namespace SimpleEngine
