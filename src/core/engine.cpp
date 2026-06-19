#include <cassert>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/engine.hpp"
#include "core/render/render_pass_manager.hpp"
#include "core/render_context.hpp"
#include "core/render_pass/example_render_pass.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#include "helpers/vulkan_helper.hpp"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <vulkan/vulkan_hpp_macros.hpp>

std::vector<const char *> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName};
std::vector<char const *> requiredLayers = {"VK_LAYER_KHRONOS_validation"};

namespace SimpleEngine {
namespace Core {

Engine::Engine() {
  RenderContext::RenderContextCreateInfo createInfo{.appName = "Simple Engine",
                                                    .inFlightFrame = 2,
                                                    .width = 800,
                                                    .height = 600};
  renderContext = RenderContext(createInfo);
  resourceManager = new ResourceManager(renderContext);

  createGraphicsPipeline();

  renderPassManager = new RenderPassManager(renderContext);
}

void Engine::createGraphicsPipeline() {
  ResourceHandle<Shader> vertexShaderHandle = resourceManager->load<Shader>(
      "test.vert", vk::ShaderStageFlagBits::eVertex);
  ResourceHandle<Shader> fragmentShaderHandle = resourceManager->load<Shader>(
      "test.frag", vk::ShaderStageFlagBits::eFragment);

  vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = vertexShaderHandle->getShaderModule(),
      .pName = "vertMain"};

  vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = fragmentShaderHandle->getShaderModule(),
      .pName = "fragMain"};

  vk::PipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStageInfo,
                                                      fragmentShaderStageInfo};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleList};

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  vk::PipelineViewportStateCreateInfo viewportState{
      .viewportCount = 1,
      .pViewports = &renderContext.viewport,
      .scissorCount = 1,
      .pScissors = &renderContext.scissor};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasClamp = vk::False,
      .lineWidth = 1.0f};

  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .logicOp = vk::LogicOp::eCopy,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  vk::PipelineDepthStencilStateCreateInfo depthStencil{
      .depthTestEnable = vk::False,
      .depthWriteEnable = vk::False,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0,
                                                  .pushConstantRangeCount = 0};

  vk::PipelineLayout pipelineLayout =
      renderContext.device.createPipelineLayout(pipelineLayoutInfo);

  vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
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
      .layout = pipelineLayout,
      .renderPass = nullptr};

  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &renderContext.swapChainSurfaceFormat.format,
      .depthAttachmentFormat = vk::Format::eD32Sfloat};

  vk::StructureChain<vk::GraphicsPipelineCreateInfo,
                     vk::PipelineRenderingCreateInfo>
      pipelineCreateInfoChain = {graphicsPipelineCreateInfo,
                                 pipelineRenderingCreateInfo};

  auto [result, pipelines] = renderContext.device.createGraphicsPipelines(
      nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

  if (result == vk::Result::eSuccess) {
    vk::Pipeline pipeline = pipelines[0];
    renderContext.graphicsPipeline = pipeline;
  }
}

void Engine::renderFrame() {
  auto fenceResult = renderContext.device.waitForFences(
      renderContext.inFlightFences[renderContext.frameIndex], vk::True,
      UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error(
        "Engine::renderFrame::ERROR: Failed to wait for render fence.");
  }

  auto [result, imageIndex] = renderContext.device.acquireNextImageKHR(
      renderContext.swapChain, UINT64_MAX,
      renderContext.presentCompleteSemaphores[renderContext.frameIndex],
      nullptr);

  if (result == vk::Result::eErrorOutOfDateKHR) {
    return;
  }
  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error(
        "Engine::renderFrame::ERROR: Failed to acquire next swap chain image.");
  }

  renderContext.device.resetFences(
      renderContext.inFlightFences[renderContext.frameIndex]);

  vk::CommandBuffer commandBuffer =
      renderContext.commandBuffers[renderContext.frameIndex];
  commandBuffer.reset();
  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  renderPassManager->execute(commandBuffer);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[imageIndex],
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageAspectFlagBits::eColor);

  vk::ImageCopy copyRegion{
      .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .extent = vk::Extent3D(renderContext.swapChainExtent.width,
                             renderContext.swapChainExtent.height, 1)};

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer,
      renderPassManager->getRenderPass("ExampleRenderPass")
          ->getRenderTarget()
          ->getColorImage(0),
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);

  commandBuffer.copyImage(renderPassManager->getRenderPass("ExampleRenderPass")
                              ->getRenderTarget()
                              ->getColorImage(0),
                          vk::ImageLayout::eTransferSrcOptimal,
                          renderContext.swapChainImages[imageIndex],
                          vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[imageIndex],
      vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::ImageAspectFlagBits::eColor);

  commandBuffer.end();

  vk::PipelineStageFlags waitDestinationStageMask =
      vk::PipelineStageFlagBits::eColorAttachmentOutput;
  vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &renderContext.presentCompleteSemaphores[renderContext.frameIndex],
      .pWaitDstStageMask = &waitDestinationStageMask,
      .commandBufferCount = 1,
      .pCommandBuffers = &commandBuffer,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = &renderContext.renderFinishedSemaphores[imageIndex]};

  vk::Result submitResult = renderContext.graphicsQueue.submit(
      1, &submitInfo, renderContext.inFlightFences[renderContext.frameIndex]);

  vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderContext.renderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &renderContext.swapChain,
      .pImageIndices = &imageIndex};

  vk::Result presentResult =
      renderContext.graphicsQueue.presentKHR(presentInfoKHR);
  if (presentResult == vk::Result::eSuboptimalKHR ||
      result == vk::Result::eErrorOutOfDateKHR) {
    // TODO: This is caused by resizing window => Swapchain outdate => Need
    // recreating
  } else {
    assert(presentResult == vk::Result::eSuccess);
  }

  renderContext.frameIndex =
      (renderContext.frameIndex + 1) % renderContext.inFlightFrame;
}

void Engine::mainLoop() {
  while (!glfwWindowShouldClose(renderContext.window)) {
    glfwPollEvents();
    renderFrame();
  }

  renderContext.device.waitIdle();
}

void Engine::setupRenderPasses() {
  renderPassManager->addRenderPass<ExampleRenderPass>("ExampleRenderPass");
}

void Engine::run() {
  std::cout << "App run\n";
  setupRenderPasses();
  mainLoop();
}
} // namespace Core
} // namespace SimpleEngine
