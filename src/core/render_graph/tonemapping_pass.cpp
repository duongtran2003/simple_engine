#include "core/render_graph/tonemapping_pass.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

constexpr int MAX_GRAPH_RESOURCE_SAMPLERS = 4;

namespace SimpleEngine {
namespace Core {
TonemappingPass::TonemappingPass(const std::string &name, CreateInfo createInfo,
                                 const RenderContext &context,
                                 ResourceManager &resourceManager)
    : RenderPass(name, context, resourceManager) {
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
      .rendering = createInfo.rendering};

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

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = static_cast<uint32_t>(layouts.size()),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}

void TonemappingPass::execute(vk::CommandBuffer &commandBuffer,
                              std::vector<Entity *> &renderObjects) {
  colorAttachment->transitionLayout(commandBuffer, context.frameIndex,
                                    vk::ImageLayout::eColorAttachmentOptimal,
                                    false);

  for (const auto &[inputName, input] : getInputs()) {
    input->transitionLayout(commandBuffer, context.frameIndex,
                            vk::ImageLayout::eShaderReadOnlyOptimal, true);
  }

  vk::RenderingAttachmentInfoKHR colorAttachment{
      .imageView = this->colorAttachment->getView(context.frameIndex),
      .imageLayout = this->colorAttachment->getLayout(context.frameIndex),
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})};

  vk::RenderingInfoKHR renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0},
                     .extent = {.width = this->colorAttachment->getWidth(),
                                .height = this->colorAttachment->getHeight()}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = nullptr};

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             getGraphicsPipeline());
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);

  auto descriptorSets = context.getGlobalDescriptorSets();

  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0,
      descriptorSets.size(), descriptorSets.data(), 0, nullptr);

  commandBuffer.draw(6, 1, 0, 0);
  commandBuffer.endRendering();
}

void TonemappingPass::init(const CreateInfo &createInfo) {
  createGraphicsPipeline(createInfo);
}
} // namespace Core
} // namespace SimpleEngine
