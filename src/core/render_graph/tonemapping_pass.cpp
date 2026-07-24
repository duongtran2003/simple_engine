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

  // This is only the global sets, each pass should have their own local sets as
  // well (for example, one pass can have a specific set to sample input)

  vk::DescriptorSetLayoutBinding bindlessResourceBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = MAX_GRAPH_RESOURCE_SAMPLERS,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr};

  std::array<vk::DescriptorSetLayoutBinding, 1> bindings = {
      bindlessResourceBinding};

  std::array<vk::DescriptorBindingFlagsEXT, 1> flags = {
      vk::DescriptorBindingFlagBitsEXT::ePartiallyBound |
      vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind |
      vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending};
  vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT bindingFlags = {
      .bindingCount = bindings.size(), .pBindingFlags = flags.data()};

  vk::DescriptorSetLayoutCreateInfo bindlessLayoutInfo = {
      .pNext = &bindingFlags,
      .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
      .bindingCount = bindings.size(),
      .pBindings = bindings.data()};

  resourceDescriptorSetLayout =
      context.device.createDescriptorSetLayout(bindlessLayoutInfo);

  vk::DescriptorPoolSize resourcePoolSize = {
      .type = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = MAX_GRAPH_RESOURCE_SAMPLERS * context.inFlightFrame};

  std::array poolSizes = {resourcePoolSize};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet |
               vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
      .maxSets = context.inFlightFrame,
      .poolSizeCount = poolSizes.size(),
      .pPoolSizes = poolSizes.data()};

  resourceDescriptorPool =
      context.device.createDescriptorPool(poolInfo, nullptr);

  std::vector<vk::DescriptorSetLayout> resourceDescriptorSetLayouts(
      context.inFlightFrame, resourceDescriptorSetLayout);

  vk::DescriptorSetAllocateInfo allocateInfo{
      .descriptorPool = resourceDescriptorPool,
      .descriptorSetCount = context.inFlightFrame,
      .pSetLayouts = resourceDescriptorSetLayouts.data()};

  resourceDescriptorSets = context.device.allocateDescriptorSets(allocateInfo);

  size_t inputIndex = 0;
  for (const auto &[inputName, input] : getInputs()) {
    for (uint32_t i = 0; i < context.inFlightFrame; i++) {
      vk::DescriptorImageInfo imageInfo{
          .sampler = input->getSampler(),
          .imageView = input->getView(i),
          .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

      vk::WriteDescriptorSet descriptorWrite{
          .dstSet = resourceDescriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = static_cast<uint32_t>(inputIndex),
          .descriptorCount = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo = &imageInfo};

      context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
    inputIndex++;
  }

  std::array<vk::DescriptorSetLayout, 3> layouts = {
      context.descriptorSetLayout, context.bindlessDescriptorSetLayout,
      resourceDescriptorSetLayout};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = layouts.size(),
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

  size_t inputIndex = 0;
  for (const auto &[inputName, input] : getInputs()) {
    for (uint32_t i = 0; i < context.inFlightFrame; i++) {
      vk::DescriptorImageInfo imageInfo{
          .sampler = input->getSampler(),
          .imageView = input->getView(i),
          .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

      vk::WriteDescriptorSet descriptorWrite{
          .dstSet = resourceDescriptorSets[i],
          .dstBinding = 0,
          .dstArrayElement = static_cast<uint32_t>(inputIndex),
          .descriptorCount = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .pImageInfo = &imageInfo};

      context.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
    inputIndex++;
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

  std::array<vk::DescriptorSet, 3> descriptorSets = {
      context.descriptorSets[context.frameIndex],
      context.bindlessDescriptorSets,
      resourceDescriptorSets[context.frameIndex]};

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
