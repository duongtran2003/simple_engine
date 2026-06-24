#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>
#include <glm/trigonometric.hpp>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/camera.hpp"
#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/engine.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/input/key_code.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_handle.hpp"
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

namespace SimpleEngine {
namespace Core {

Engine::Engine() {
  RenderContext::RenderContextCreateInfo createInfo{.appName = "Simple Engine",
                                                    .inFlightFrame = 2,
                                                    .width = 1280,
                                                    .height = 720};
  renderContext = RenderContext(createInfo);
  resourceManager = new ResourceManager(renderContext);
  input = new Input(renderContext);
  camera = new Camera(*input);
  camera->setVFov(60.0f);
  float aspect = (float)renderContext.width / (float)renderContext.height;
  camera->setAspectRatio(aspect);

  createUniformBuffers();
  createDescriptorPool();
  createDescriptorSetLayout();
  createDescriptorSets();
  createGraphicsPipeline();
  renderGraph = new RenderGraph(renderContext);

  lastFrameTime = std::chrono::high_resolution_clock::now();
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

  vk::VertexInputBindingDescription vertexInputBindingDescription{
      .binding = 0,
      .stride = sizeof(Mesh::Vertex),
      .inputRate = vk::VertexInputRate::eVertex};
  std::array<vk::VertexInputAttributeDescription, 3>
      vertexInputAttributeDescriptions = {};
  vertexInputAttributeDescriptions[0] = {.location = 0,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32B32Sfloat,
                                         .offset =
                                             offsetof(Mesh::Vertex, position)};
  vertexInputAttributeDescriptions[1] = {.location = 1,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32B32Sfloat,
                                         .offset =
                                             offsetof(Mesh::Vertex, normal)};
  vertexInputAttributeDescriptions[2] = {.location = 2,
                                         .binding = 0,
                                         .format = vk::Format::eR32G32Sfloat,
                                         .offset = offsetof(Mesh::Vertex, uv)};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
      .vertexBindingDescriptionCount = 1,
      .pVertexBindingDescriptions = &vertexInputBindingDescription,
      .vertexAttributeDescriptionCount =
          vertexInputAttributeDescriptions.size(),
      .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()};
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
      .frontFace = vk::FrontFace::eCounterClockwise,
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
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLess,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = 1,
      .pSetLayouts = &renderContext.descriptorSetLayout,
      .pushConstantRangeCount = 0};

  vk::PipelineLayout pipelineLayout =
      renderContext.device.createPipelineLayout(pipelineLayoutInfo);

  renderContext.pipelineLayout = pipelineLayout;

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

  renderGraph->execute(commandBuffer);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[imageIndex],
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageAspectFlagBits::eColor);

  GraphResource *outputResource = renderGraph->getOutputResource();
  outputResource->transitionLayout(commandBuffer,
                                   vk::ImageLayout::eTransferSrcOptimal);

  vk::ImageCopy copyRegion{
      .srcSubresource = {.aspectMask = outputResource->getAspectMask(),
                         .mipLevel = 0,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .mipLevel = 0,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .extent = {.width = renderContext.swapChainExtent.width,
                 .height = renderContext.swapChainExtent.height,
                 .depth = 1}};

  commandBuffer.copyImage(outputResource->getImage(),
                          outputResource->getLayout(),
                          renderContext.swapChainImages[imageIndex],
                          vk::ImageLayout::eTransferDstOptimal, copyRegion);

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

  vk::PresentInfoKHR presentInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = &renderContext.renderFinishedSemaphores[imageIndex],
      .swapchainCount = 1,
      .pSwapchains = &renderContext.swapChain,
      .pImageIndices = &imageIndex};

  vk::Result presentResult =
      renderContext.graphicsQueue.presentKHR(presentInfo);

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

void Engine::updateFrameTime() {
  auto currentFrameTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed = currentFrameTime - lastFrameTime;
  deltaTime = elapsed.count();

  lastFrameTime = currentFrameTime;
  if (deltaTime > 0.1f) {
    deltaTime = 0.1f;
  }
}

void Engine::mainLoop() {
  while (!glfwWindowShouldClose(renderContext.window)) {
    updateFrameTime();
    glfwPollEvents();
    input->update();
    handleInput(deltaTime);
    camera->update(deltaTime);
    renderFrame();

    input->clearMouseDelta();
  }

  renderContext.device.waitIdle();
}

void Engine::setupExampleRenderGraph() {
  GraphResource *resource = new GraphResource(
      "final_color", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height,
      renderContext.swapChainSurfaceFormat.format, vk::ImageLayout::eUndefined,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc,
      renderContext);
  renderGraph->addResource(resource);
  renderGraph->setOutputResource("final_color");

  GraphResource *depthResource = new GraphResource(
      "depth_image", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height, vk::Format::eD32Sfloat,
      vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eDepth,
      vk::ImageUsageFlagBits::eDepthStencilAttachment, renderContext);
  renderGraph->addResource(depthResource);

  RenderPass *pass = new RenderPass("example_pass", renderContext);
  pass->addOutput("final_color");
  const auto passCallback = [&](vk::CommandBuffer &commandBuffer) {
    GraphResource *finalColor = renderGraph->getResource("final_color");
    finalColor->transitionLayout(commandBuffer,
                                 vk::ImageLayout::eColorAttachmentOptimal);
    GraphResource *depthImage = renderGraph->getResource("depth_image");
    depthImage->transitionLayout(
        commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::RenderingAttachmentInfoKHR colorAttachment{
        .imageView = finalColor->getView(),
        .imageLayout = finalColor->getLayout(),
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue =
            vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})};

    vk::RenderingAttachmentInfoKHR depthAttachment{
        .imageView = depthImage->getView(),
        .imageLayout = depthImage->getLayout(),
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

    vk::RenderingInfoKHR renderingInfo{
        .renderArea = {.offset = {.x = 0, .y = 0},
                       .extent = {.width = finalColor->getWidth(),
                                  .height = finalColor->getHeight()}},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthAttachment};

    commandBuffer.beginRendering(renderingInfo);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               renderContext.graphicsPipeline);
    commandBuffer.setViewport(0, renderContext.viewport);
    commandBuffer.setScissor(0, renderContext.scissor);

    for (Entity *e : renderObjects) {
      auto *mesh = e->getComponent<MeshComponent>();
      auto *transform = e->getComponent<TransformComponent>();

      if (!mesh || !mesh->getMesh()->isLoaded()) {
        continue;
      }

      auto meshResource = mesh->getMesh().get();

      vk::Buffer vertexBuffers[] = {meshResource->getVertexBuffer()};
      vk::DeviceSize offsets[] = {0};

      float spinSpeedY = glm::radians(15.0f);
      glm::quat deltaY =
          glm::angleAxis(spinSpeedY * deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));

      glm::quat frameRotation = deltaY;

      glm::quat currentRotation = transform->getRotation();
      transform->setRotation(frameRotation * currentRotation);

      updateUniformBuffer(renderContext.frameIndex,
                          transform->getTransformMatrix());

      commandBuffer.bindDescriptorSets(
          vk::PipelineBindPoint::eGraphics, renderContext.pipelineLayout, 0, 1,
          &renderContext.descriptorSets[renderContext.frameIndex], 0, nullptr);
      commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
      commandBuffer.bindIndexBuffer(meshResource->getIndexBuffer(), 0,
                                    vk::IndexType::eUint32);

      commandBuffer.drawIndexed(meshResource->getIndexCount(), 1, 0, 0, 0);
    }

    commandBuffer.endRendering();
  };
  pass->setExecuteCallbackFn(passCallback);
  renderGraph->addPass(pass);
  renderGraph->compile();
}

void Engine::createUniformBuffers() {
  vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
  for (size_t i = 0; i < renderContext.inFlightFrame; i++) {
    const auto &[buffer, memory] = Helper::VulkanHelper::createBuffer(
        bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
            vk::MemoryPropertyFlagBits::eHostCoherent,
        renderContext);

    UboBuffer uboBuffer{
        .buffer = std::move(buffer),
        .memory = std::move(memory),
    };
    uboBuffer.mapped =
        renderContext.device.mapMemory(uboBuffer.memory, 0, bufferSize);
    uniformBuffers.push_back(uboBuffer);
  }
}

void Engine::createDescriptorPool() {
  vk::DescriptorPoolSize uniformPoolSize(vk::DescriptorType::eUniformBuffer,
                                         renderContext.inFlightFrame);

  vk::DescriptorPoolSize imageSamplerPoolSize(
      vk::DescriptorType::eCombinedImageSampler, renderContext.inFlightFrame);

  std::array poolSizes = {uniformPoolSize, imageSamplerPoolSize};

  vk::DescriptorPoolCreateInfo poolInfo{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = renderContext.inFlightFrame,
      .poolSizeCount = poolSizes.size(),
      .pPoolSizes = poolSizes.data()};

  renderContext.descriptorPool =
      renderContext.device.createDescriptorPool(poolInfo, nullptr);
}

void Engine::createDescriptorSetLayout() {
  vk::DescriptorSetLayoutBinding uboLayoutBinding{
      .binding = 0,
      .descriptorType = vk::DescriptorType::eUniformBuffer,
      .descriptorCount = 1,
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .pImmutableSamplers = nullptr};

  vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = 1,
                                               .pBindings = &uboLayoutBinding};

  renderContext.descriptorSetLayout =
      renderContext.device.createDescriptorSetLayout(layoutInfo);
}

void Engine::createDescriptorSets() {
  std::vector<vk::DescriptorSetLayout> layouts(
      renderContext.inFlightFrame, renderContext.descriptorSetLayout);

  vk::DescriptorSetAllocateInfo allocateInfo{
      .descriptorPool = renderContext.descriptorPool,
      .descriptorSetCount = renderContext.inFlightFrame,
      .pSetLayouts = layouts.data()};

  renderContext.descriptorSets =
      renderContext.device.allocateDescriptorSets(allocateInfo);

  vk::DescriptorBufferInfo bufferInfo{.offset = 0,
                                      .range = sizeof(UniformBufferObject)};

  for (size_t i = 0; i < renderContext.inFlightFrame; i++) {
    bufferInfo.buffer = uniformBuffers[i].buffer;

    vk::WriteDescriptorSet descriptorWrite{
        .dstSet = renderContext.descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo};

    renderContext.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
  }
}

void Engine::updateUniformBuffer(uint32_t currentFrame, glm::mat4 model) {
  UniformBufferObject ubo{};
  ubo.model = model;
  ubo.normalModel = glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
  ubo.view = camera->getCamera()->getViewMatrix();
  ubo.proj = camera->getCamera()->getProjectionMatrix();

  ubo.lightDirection = glm::vec3(0.0f, -5.0f, -5.0f);
  ubo.objectColor = glm::vec3(0.7f, 1.0f, 0.82f);

  memcpy(uniformBuffers[currentFrame].mapped, &ubo, sizeof(ubo));
}

void Engine::initRenderObjectsList() {
  ResourceHandle<Mesh> meshResource = resourceManager->load<Mesh>(
      "model_damaged_helmet",
      "resources/models/damaged_helmet/DamagedHelmet.glb");
  uint32_t verticesCount = meshResource.get()->getVertexCount();
  std::cout << "vertices count: " << verticesCount << "\n";

  Entity *newEntity = new Entity("helmet");

  newEntity->addComponent<MeshComponent>();
  newEntity->getComponent<MeshComponent>()->setMesh(meshResource);
  newEntity->addComponent<TransformComponent>();
  auto *transform = newEntity->getComponent<TransformComponent>();
  transform->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
  transform->setScale(glm::vec3(1.0f, 1.0f, 1.0f));

  glm::vec3 axis = {1.0f, 0.0f, 0.0f};
  float angle = glm::radians(90.0f);
  glm::quat rotQuat = glm::angleAxis(angle, axis);
  glm::quat rot = transform->getRotation();
  rot = rotQuat * rot;
  transform->setRotation(rot);

  renderObjects.push_back(newEntity);
}

void Engine::handleInput(float delta) {
  if (input->isKeyJustPressed(Key::Escape) ||
      input->isKeyJustPressed(Key::CapsLock)) {
    glfwSetWindowShouldClose(renderContext.window, true);
  }

  if (input->isKeyJustPressed(Key::L)) {
    input->toggleMouseLock();
  }
}

void Engine::run() {
  std::cout << "App run\n";
  initRenderObjectsList();
  setupExampleRenderGraph();
  mainLoop();
}
} // namespace Core
} // namespace SimpleEngine
