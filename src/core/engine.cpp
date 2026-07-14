#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <glm/detail/qualifier.hpp>
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
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/camera.hpp"
#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/engine.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/material.hpp"
#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#include "core/resource/texture.hpp"
#include "core/system/culling_system.hpp"
#include "enums/input.hpp"
#include "helpers/asset_loader.hpp"
#include "helpers/vulkan_helper.hpp"
#include "ui/camera_ui.hpp"
#include "ui/imgui_vulkan.hpp"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <vulkan/vulkan_hpp_macros.hpp>

namespace SimpleEngine {
namespace Core {

RenderContext::UniformBufferObject ubo{};

Engine::Engine() {
  RenderContext::RenderContextCreateInfo createInfo{.appName = "Simple Engine",
                                                    .inFlightFrame = 2,
                                                    .width = 1280,
                                                    .height = 720};
  renderContext = RenderContext(createInfo);
  renderContext.setMsaaSamples(vk::SampleCountFlagBits::e1);
  resourceManager = new ResourceManager(renderContext);
  input = new Input(renderContext);
  camera = new Camera(*input);
  camera->setVFov(60.0f);
  float aspect = (float)renderContext.width / (float)renderContext.height;
  camera->setAspectRatio(aspect);

  cullingSystem = new CullingSystem(camera);

  createGraphicsPipeline();

  imGui = new UI::ImGuiVulkan(renderContext, *resourceManager);
  imGui->init(renderContext.swapChainExtent.width,
              renderContext.swapChainExtent.height);
  imGui->initResources();
  input->setImGui(imGui);
  cameraUI = new UI::CameraUI(*camera);

  renderGraph = new RenderGraph(renderContext);

  lastFrameTime = std::chrono::high_resolution_clock::now();
}

void Engine::createGraphicsPipeline() {
  ResourceHandle<Shader> vertexShaderHandle = resourceManager->load<Shader>(
      "test_vert", vk::ShaderStageFlagBits::eVertex, "shaders/test.vert.spv");
  ResourceHandle<Shader> fragmentShaderHandle = resourceManager->load<Shader>(
      "test_frag", vk::ShaderStageFlagBits::eFragment, "shaders/test.frag.spv");

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
  std::array<vk::VertexInputAttributeDescription, 4>
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
  vertexInputAttributeDescriptions[3] = {
      .location = 3,
      .binding = 0,
      .format = vk::Format::eR32G32B32A32Sfloat,
      .offset = offsetof(Mesh::Vertex, tangent)};

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

  std::array<vk::DescriptorSetLayout, 3> layouts = {
      renderContext.descriptorSetLayout,
      renderContext.bindlessDescriptorSetLayout,
      renderContext.ssboDescriptorSetLayout};

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
      commandBuffer, renderContext.swapChainImages[imageIndex], 1, 0,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageAspectFlagBits::eColor);

  GraphResource *outputResource = renderGraph->getOutputResource();
  bool fromLastLayout = true;
  outputResource->transitionLayout(
      commandBuffer, vk::ImageLayout::eTransferSrcOptimal, fromLastLayout);

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
      commandBuffer, renderContext.swapChainImages[imageIndex], 1, 0,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageAspectFlagBits::eColor);

  // Handle UI here, abstract later
  imGui->beginFrame();
  cameraUI->render();
  imGui->endFrame(renderContext.frameIndex);
  imGui->drawFrame(commandBuffer, renderContext.swapChainImageViews[imageIndex],
                   renderContext.frameIndex);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[imageIndex], 1, 0,
      vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR,
      vk::ImageAspectFlagBits::eColor);

  commandBuffer.end();

  vk::PipelineStageFlags waitDestinationStageMask =
      vk::PipelineStageFlagBits::eTransfer;

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

  RenderPass *pass = new RenderPass("example_pass", renderContext);
  pass->addOutput("final_color");
  const auto passCallback = [&](vk::CommandBuffer &commandBuffer) {
    GraphResource *finalColor = renderGraph->getResource("final_color");
    bool fromLastLayout = false;
    finalColor->transitionLayout(commandBuffer,
                                 vk::ImageLayout::eColorAttachmentOptimal,
                                 fromLastLayout);
    GraphResource *depthImage = renderGraph->getResource("depth_image");
    depthImage->transitionLayout(
        commandBuffer, vk::ImageLayout::eDepthStencilAttachmentOptimal,
        fromLastLayout);

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

    std::array<vk::DescriptorSet, 3> descriptorSets = {
        renderContext.descriptorSets[renderContext.frameIndex],
        renderContext.bindlessDescriptorSets,
        renderContext.ssboDescriptorSets[renderContext.frameIndex]};
    commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, renderContext.pipelineLayout, 0,
        descriptorSets.size(), descriptorSets.data(), 0, nullptr);

    ubo.view = camera->getCamera()->getViewMatrix();
    ubo.proj = camera->getCamera()->getProjectionMatrix();
    ubo.cameraPos = camera->getTransform()->getPosition();
    memcpy(renderContext.getCurrentFrameUniformBufferPtr(), &ubo, sizeof(ubo));

    ObjectData *ssboDataArray = static_cast<ObjectData *>(
        renderContext.getCurrentFrameStorageBufferPtr());

    for (size_t i = 0; i < renderObjects.size(); i++) {
      Entity *e = renderObjects[i];

      auto *mesh = e->getComponent<MeshComponent>();
      auto *transform = e->getComponent<TransformComponent>();

      if (!mesh || !mesh->getMesh()->isLoaded()) {
        continue;
      }

      auto meshResource = mesh->getMesh().get();

      vk::Buffer vertexBuffers[] = {meshResource->getVertexBuffer()};
      vk::DeviceSize offsets[] = {0};

      const auto &mat = mesh->getMaterial();

      ssboDataArray[i].model = transform->getTransformMatrix();
      ssboDataArray[i].normalModel = transform->getNormalTransformMatrix();
      ssboDataArray[i].pbrBaseColorFactor = mat.getPbrBaseColorFactor();

      uint32_t albedoIndex = mat.hasAlbedo() ? mat.getAlbedo().index : 0;
      uint32_t normalIndex = mat.hasNormal() ? mat.getNormal().index : 0;

      PushConstants pushConstant{.albedoIndex = albedoIndex,
                                 .normalIndex = normalIndex,
                                 .uniformIndex = static_cast<uint32_t>(i)};

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

std::vector<Entity *> processNodesList(std::vector<RawSceneNode> &nodes,
                                       ResourceManager *resourceManager,
                                       RenderContext &renderContext) {
  std::vector<Entity *> entities;

  std::unordered_map<std::string, int> textureSlots;
  int nextSlot = 0;

  for (const auto &node : nodes) {
    Entity *e = new Entity(node.name);

    e->addComponent<TransformComponent>();
    auto transform = e->getComponent<TransformComponent>();
    transform->setPosition(node.translation)
        ->setRotation(node.rotation)
        ->setScale(node.scale);

    e->addComponent<MeshComponent>();
    ResourceHandle<Mesh> meshResource =
        resourceManager->load<Mesh>(node.name, node.vertices, node.indices);

    auto mesh = e->getComponent<MeshComponent>();
    mesh->setMesh(meshResource);

    Material material;
    const RawTexture &rawAlbedo = node.textures[static_cast<size_t>(
        RawSceneNode::TextureIndexer::Albedo)];
    if (rawAlbedo.isValid) {
      material.setPbrBaseColorFactor(
          node.textures[0].pbrProperty.baseColorFactor);

      if (!rawAlbedo.pixels.empty() || rawAlbedo.hasLoadedImage) {
        ResourceHandle<Texture> albedo =
            resourceManager->load<Texture>(rawAlbedo.name, rawAlbedo);

        auto it = textureSlots.find(albedo.getId());
        if (it == textureSlots.end()) {
          nextSlot += 1;
          textureSlots[albedo.getId()] = nextSlot;
        }

        int slot = textureSlots[albedo.getId()];
        material.setAlbedo(
            {.index = static_cast<uint32_t>(slot), .handle = albedo});

        material.registerAlbedo(renderContext.bindlessDescriptorSets,
                                textureSlots.find(albedo.getId())->second,
                                renderContext);
      }
    }

    const RawTexture &rawNormal = node.textures[static_cast<size_t>(
        RawSceneNode::TextureIndexer::Normal)];
    if (rawNormal.isValid) {
      ResourceHandle<Texture> normal =
          resourceManager->load<Texture>(rawNormal.name, rawNormal);

      auto it = textureSlots.find(normal.getId());
      if (it == textureSlots.end()) {
        nextSlot += 1;
        textureSlots[normal.getId()] = nextSlot;
      }

      int slot = textureSlots[normal.getId()];
      material.setNormal(
          {.index = static_cast<uint32_t>(slot), .handle = normal});

      material.registerNormal(renderContext.bindlessDescriptorSets,
                              textureSlots.find(normal.getId())->second,
                              renderContext);
    }

    mesh->setMaterial(material);
    entities.emplace_back(e);
  }

  return entities;
}

std::vector<Entity *> loadSponza(ResourceManager *resourceManager,
                                 RenderContext &renderContext, Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/scenes/sponza";
  const std::string sceneName = "Sponza";

  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({-7.0f, 4.5f, -0.36f});

  glm::vec3 cameraRotateAxis = {0.0f, 1.0f, 0.0f};
  glm::quat cameraRot = glm::angleAxis(glm::radians(-90.0f), cameraRotateAxis);
  camera->getTransform()->setRotation(cameraRot *
                                      camera->getTransform()->getRotation());

  ubo.directionalLightDirection = glm::vec3(8.0f, -12.0f, 6.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-7.0f, 4.5f, -0.36f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext);

  return entities;
}

std::vector<Entity *> loadOrientationTest(ResourceManager *resourceManager,
                                          RenderContext &renderContext,
                                          Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/scenes/orientation_test";
  const std::string sceneName = "OrientationTest";

  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 5.0f});

  ubo.directionalLightDirection = glm::vec3(-3.0f, -5.0f, -5.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-3.0f, 5.0f, -3.0f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext);

  return entities;
}

std::vector<Entity *> loadBall(ResourceManager *resourceManager,
                               RenderContext &renderContext, Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/models/baseball_01_4k";
  const std::string sceneName = "baseball_01_4k";

  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 5.0f});

  ubo.directionalLightDirection = glm::vec3(-3.0f, -5.0f, -5.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-3.0f, 5.0f, -3.0f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext);

  for (const auto &e : entities) {
    e->getComponent<TransformComponent>()->setScale({20.0f, 20.0f, 20.0f});
  }

  return entities;
}

std::vector<Entity *> loadScene(ResourceManager *resourceManager,
                                RenderContext &renderContext, Camera *camera) {

  std::vector<Entity *> entities;
  entities = loadSponza(resourceManager, renderContext, camera);
  return entities;
}

void Engine::initRenderObjectsList() {
  renderObjects = loadScene(resourceManager, renderContext, camera);
  renderContext.createStorageBuffers<ObjectData>(renderObjects.size());
}

void Engine::handleInput(float delta) {
  using eKey = Enums::Input::Key;
  if (input->isKeyJustPressed(eKey::Escape) ||
      input->isKeyJustPressed(eKey::CapsLock)) {
    glfwSetWindowShouldClose(renderContext.window, true);
  }

  if (input->isKeyJustPressed(eKey::L)) {
    input->toggleMouseLock();
  }
}

void Engine::run() {
  std::cout << "Engine::run::INFO: Engine's running\n";
  initRenderObjectsList();
  setupExampleRenderGraph();
  mainLoop();
}
} // namespace Core
} // namespace SimpleEngine
