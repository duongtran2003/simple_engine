#include "core/render_graph/skybox_pass.hpp"
#include "core/entity/entity.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace SimpleEngine {
namespace Core {
SkyboxPass::SkyboxPass(const std::string &name, CreateInfo createInfo,
                       const RenderContext &context,
                       ResourceManager &resourceManager)
    : RenderPass(name, context, resourceManager) {
  CreateInfo mainPassCreateInfo{
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
      .rendering = createInfo.rendering};

  init(mainPassCreateInfo);
}

SkyboxPass::~SkyboxPass() {
  // TODO
}

void SkyboxPass::init(const CreateInfo &createInfo) {
  createGraphicsPipeline(createInfo);
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

  // This is only the global sets, each pass should have their own local sets as
  // well (for example, one pass can have a specific set to sample input)
  std::array<vk::DescriptorSetLayout, 2> layouts = {
      context.descriptorSetLayout, context.bindlessDescriptorSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = layouts.size(),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 0,
      .pPushConstantRanges = nullptr};

  return context.device.createPipelineLayout(pipelineLayoutInfo);
}

void SkyboxPass::execute(vk::CommandBuffer &commandBuffer,
                         std::vector<Entity *> &renderObjects) {
  bool fromLastLayout = true;
  colorAttachment->transitionLayout(
      commandBuffer, vk::ImageLayout::eColorAttachmentOptimal, fromLastLayout);

  depthAttachment->transitionLayout(
      commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
      fromLastLayout);

  vk::RenderingAttachmentInfoKHR colorAttachment{
      .imageView = this->colorAttachment->getView(),
      .imageLayout = this->colorAttachment->getLayout(),
      .loadOp = vk::AttachmentLoadOp::eLoad,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue =
          vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})};

  vk::RenderingAttachmentInfoKHR depthAttachment{
      .imageView = this->depthAttachment->getView(),
      .imageLayout = this->depthAttachment->getLayout(),
      .loadOp = vk::AttachmentLoadOp::eLoad,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

  vk::RenderingInfoKHR renderingInfo{
      .renderArea = {.offset = {.x = 0, .y = 0},
                     .extent = {.width = this->colorAttachment->getWidth(),
                                .height = this->colorAttachment->getHeight()}},
      .layerCount = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachment,
      .pDepthAttachment = &depthAttachment};

  commandBuffer.beginRendering(renderingInfo);

  commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             getGraphicsPipeline());
  commandBuffer.setViewport(0, context.viewport);
  commandBuffer.setScissor(0, context.scissor);

  std::array<vk::DescriptorSet, 2> descriptorSets = {
      context.descriptorSets[context.frameIndex],
      context.bindlessDescriptorSets};
  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, graphicsPipelineLayout, 0,
      descriptorSets.size(), descriptorSets.data(), 0, nullptr);

  commandBuffer.draw(36, 1, 0, 0);
  commandBuffer.endRendering();
}
} // namespace Core
} // namespace SimpleEngine
