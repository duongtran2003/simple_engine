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
#include <tuple>
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
#include "helpers/model_loader.hpp"
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
  renderContext.setMsaaSamples(vk::SampleCountFlagBits::e16);
  resourceManager = new ResourceManager(renderContext);
  input = new Input(renderContext);
  camera = new Camera(*input);
  camera->setVFov(60.0f);
  float aspect = (float)renderContext.width / (float)renderContext.height;
  camera->setAspectRatio(aspect);

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
      .rasterizationSamples = renderContext.msaaSamples,
      .sampleShadingEnable = vk::True,
      .minSampleShading = .2f};

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

  std::array<vk::DescriptorSetLayout, 2> layouts = {
      renderContext.descriptorSetLayout,
      renderContext.bindlessDescriptorSetLayout};

  vk::PushConstantRange pushConstantRange{
      .stageFlags =
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      .offset = 0,
      .size = sizeof(PushConstants)};

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
      .setLayoutCount = layouts.size(),
      .pSetLayouts = layouts.data(),
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &pushConstantRange};

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
      vk::SampleCountFlagBits::e1, renderContext);
  renderGraph->addResource(resource);
  renderGraph->setOutputResource("final_color");

  GraphResource *depthResource = new GraphResource(
      "depth_image", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height, vk::Format::eD32Sfloat,
      vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eDepth,
      vk::ImageUsageFlagBits::eDepthStencilAttachment,
      renderContext.msaaSamples, renderContext);
  renderGraph->addResource(depthResource);

  GraphResource *colorResource = new GraphResource(
      "color_image", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height,
      renderContext.swapChainSurfaceFormat.format, vk::ImageLayout::eUndefined,
      vk::ImageAspectFlagBits::eColor, vk::ImageUsageFlagBits::eColorAttachment,
      renderContext.msaaSamples, renderContext);
  renderGraph->addResource(colorResource);

  RenderPass *pass = new RenderPass("example_pass", renderContext);
  pass->addOutput("final_color");
  const auto passCallback = [&](vk::CommandBuffer &commandBuffer) {
    GraphResource *finalColor = renderGraph->getResource("final_color");
    finalColor->transitionLayout(commandBuffer,
                                 vk::ImageLayout::eColorAttachmentOptimal);
    GraphResource *colorImage = renderGraph->getResource("color_image");
    colorImage->transitionLayout(commandBuffer,
                                 vk::ImageLayout::eColorAttachmentOptimal);
    GraphResource *depthImage = renderGraph->getResource("depth_image");
    depthImage->transitionLayout(
        commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::RenderingAttachmentInfoKHR colorAttachment{
        .imageView = colorResource->getView(),
        .imageLayout = colorResource->getLayout(),

        .resolveMode = vk::ResolveModeFlagBits::eAverage,
        .resolveImageView = finalColor->getView(),
        .resolveImageLayout = finalColor->getLayout(),

        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .clearValue = vk::ClearColorValue(
            std::array<float, 4>{1.0f, 0.96f, 0.89f, 1.0f})};

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

    std::array<vk::DescriptorSet, 2> descriptorSets = {
        renderContext.descriptorSets[renderContext.frameIndex],
        renderContext.bindlessDescriptorSets};
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, renderContext.pipelineLayout, 0,
        descriptorSets.size(), descriptorSets.data(), 0, nullptr);

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

      glm::mat4 model = transform->getTransformMatrix();
      RenderContext::UniformBufferObject ubo{};
      ubo.normalModel =
          glm::mat4(glm::transpose(glm::inverse(glm::mat3(model))));
      ubo.view = camera->getCamera()->getViewMatrix();
      ubo.proj = camera->getCamera()->getProjectionMatrix();

      ubo.directionalLightDirection = glm::vec3(0.0f, -5.0f, -5.0f);
      ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

      ubo.pointLightPosition = glm::vec3(-5.0f, 5.0f, -5.0f);
      ubo.pointLightColor = glm::vec3(1.0f);

      memcpy(renderContext.getCurrentFrameUniformBufferPtr(), &ubo,
             sizeof(ubo));

      PushConstants pushConstant{.modelMatrix = model,
                                 .cameraPos =
                                     camera->getTransform()->getPosition(),
                                 .meshTextureIndex = 1};

      commandBuffer.pushConstants(renderContext.pipelineLayout,
                                  vk::ShaderStageFlagBits::eVertex |
                                      vk::ShaderStageFlagBits::eFragment,
                                  0, sizeof(PushConstants), &pushConstant);
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

// Load different models, this stays here till GUI implementation
std::tuple<glm::vec3, glm::vec3, glm::quat>
loadHelmet(ResourceManager *resourceManager, MeshComponent *meshComponent) {
  Helper::ModelLoader::loadGltfMesh(
      "resources/models/damaged_helmet/DamagedHelmet.glb",
      "model_damaged_helmet", *meshComponent, *resourceManager);

  glm::vec3 position = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};
  glm::vec3 axis = {1.0f, 0.0f, 0.0f};
  float angle = glm::radians(90.0f);
  glm::quat rotQuat = glm::angleAxis(angle, axis);

  return {position, scale, rotQuat};
}

std::tuple<ResourceHandle<Mesh>, glm::vec3, glm::vec3, glm::quat>
loadCorset(ResourceManager *resourceManager) {
  ResourceHandle<Mesh> meshResource = resourceManager->load<Mesh>(
      "model_corset", "resources/models/corset/Corset.glb");

  glm::vec3 position = {0.0f, -0.8f, 0.0f};
  glm::vec3 scale = {30.0f, 30.0f, 30.0f};
  glm::vec3 axis = {0.0f, 1.0f, 0.0f};
  float angle = glm::radians(180.0f);
  glm::quat rotQuat = glm::angleAxis(angle, axis);

  return {std::move(meshResource), position, scale, rotQuat};
}

std::tuple<ResourceHandle<Mesh>, glm::vec3, glm::vec3, glm::quat>
loadDucky(ResourceManager *resourceManager) {
  ResourceHandle<Mesh> meshResource = resourceManager->load<Mesh>(
      "model_ducky", "resources/models/duck/Duck.glb");

  glm::vec3 position = {0.0f, -0.8f, 0.0f};
  glm::vec3 scale = {0.01f, 0.01f, 0.01f};
  glm::vec3 axis = {1.0f, 0.0f, 0.0f};
  float angle = glm::radians(0.0f);
  glm::quat rotQuat = glm::angleAxis(angle, axis);

  return {std::move(meshResource), position, scale, rotQuat};
}

void Engine::initRenderObjectsList() {
  Entity *newEntity = new Entity("helmet");

  newEntity->addComponent<MeshComponent>();
  auto [ePosition, eScale, eRot] =
      loadHelmet(resourceManager, newEntity->getComponent<MeshComponent>());

  newEntity->addComponent<TransformComponent>();
  auto *transform = newEntity->getComponent<TransformComponent>();
  transform->setPosition(ePosition);
  transform->setScale(eScale);

  auto *mesh = newEntity->getComponent<MeshComponent>();

  glm::quat rot = transform->getRotation();
  rot = eRot * rot;
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
  std::cout << "Engine::run::INFO: Engine's running\n";
  initRenderObjectsList();
  // setupExampleRenderGraph();
  // mainLoop();
}
} // namespace Core
} // namespace SimpleEngine
