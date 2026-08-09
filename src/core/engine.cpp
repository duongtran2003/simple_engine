#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS

#include <array>
#include <cassert>
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
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/camera.hpp"
#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/engine.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/material.hpp"
#include "core/profiler/profiler.hpp"
#include "core/raw_scene_node.hpp"
#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/graph_resource.hpp"
#include "core/render_graph/main_pass.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/render_graph/render_pass.hpp"
#include "core/render_graph/skybox_pass.hpp"
#include "core/render_graph/tonemapping_pass.hpp"
#include "core/resource/cubemap.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/texture.hpp"
#include "core/system/culling_system.hpp"
#include "enums/input.hpp"
#include "helpers/asset_loader.hpp"
#include "helpers/vulkan_helper.hpp"
#include "ui/camera_ui.hpp"
#include "ui/imgui_vulkan.hpp"
#include "ui/profiler_ui.hpp"
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
                                                    .width = 1600,
                                                    .height = 900};
  renderContext = RenderContext(createInfo);
  renderContext.setMsaaSamples(vk::SampleCountFlagBits::e1);
  resourceManager = new ResourceManager(renderContext);
  input = new Input(renderContext);
  camera = new Camera(*input);
  camera->setVFov(60.0f);
  float aspect = (float)renderContext.width / (float)renderContext.height;
  camera->setAspectRatio(aspect);

  cullingSystem = new CullingSystem(camera);

  profiler = new Profiler(renderContext);

  imGui = new UI::ImGuiVulkan(renderContext, *resourceManager);
  imGui->init(renderContext.swapChainExtent.width,
              renderContext.swapChainExtent.height);
  imGui->initResources();
  input->setImGui(imGui);
  cameraUI = new UI::CameraUI(*camera);
  profilerUI = new UI::ProfilerUI(*profiler);
}

void Engine::handleSwapchainRecreation() {
  renderContext.recreateSwapChain();
  setupExampleRenderGraph();
}

std::pair<vk::Result, uint32_t> Engine::acquireSwapChainImage() {
  auto fenceResult = renderContext.device.waitForFences(
      renderContext.inFlightFences[renderContext.frameIndex], vk::True,
      UINT64_MAX);
  if (fenceResult != vk::Result::eSuccess) {
    throw std::runtime_error(
        "Engine::acquireSwapChainImage::ERROR: Failed to wait for render fence.");
  }

  auto [result, imageIndex] = renderContext.device.acquireNextImageKHR(
      renderContext.swapChain, UINT64_MAX,
      renderContext.presentCompleteSemaphores[renderContext.frameIndex],
      nullptr);
  std::cout << "Acquired image index: " << imageIndex
           << " @ " << glfwGetTime() << "\n";

  if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR &&
      result != vk::Result::eErrorOutOfDateKHR) {
    throw std::runtime_error("Engine::acquireSwapChainImage::ERROR: Failed to "
                             "acquire next swap chain image.");
  }

  if (result != vk::Result::eErrorOutOfDateKHR) {
    renderContext.device.resetFences(
        renderContext.inFlightFences[renderContext.frameIndex]);
  }

  return {result, imageIndex};
}

void Engine::renderFrame(uint32_t renderImageIndex) {
  vk::CommandBuffer commandBuffer =
      renderContext.commandBuffers[renderContext.frameIndex];
  commandBuffer.reset();

  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  ubo.view = camera->getCamera()->getViewMatrix();
  ubo.proj = camera->getCamera()->getProjectionMatrix();
  ubo.cameraPos = camera->getTransform()->getPosition();
  memcpy(renderContext.getCurrentFrameUniformBufferPtr(), &ubo, sizeof(ubo));

  renderGraph->execute(commandBuffer, renderObjects);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[renderImageIndex], 1, 0, 1,
      0, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageAspectFlagBits::eColor);

  GraphResource *outputResource = renderGraph->getOutputResource();
  bool fromLastLayout = true;
  outputResource->transitionLayout(commandBuffer, renderContext.frameIndex,
                                   vk::ImageLayout::eTransferSrcOptimal,
                                   fromLastLayout);

  vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets, dstOffsets;

  srcOffsets[0] = vk::Offset3D(0, 0, 0);
  srcOffsets[1] =
      vk::Offset3D(static_cast<int32_t>(outputResource->getWidth()),
                   static_cast<int32_t>(outputResource->getHeight()), 1);

  dstOffsets[0] = vk::Offset3D(0, 0, 0);
  dstOffsets[1] = vk::Offset3D(
      static_cast<int32_t>(renderContext.swapChainExtent.width),
      static_cast<int32_t>(renderContext.swapChainExtent.height), 1);

  vk::ImageBlit blitRegion{
      .srcSubresource = {.aspectMask = outputResource->getAspectMask(),
                         .mipLevel = 0,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .srcOffsets = srcOffsets,
      .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .mipLevel = 0,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .dstOffsets = dstOffsets};

  commandBuffer.blitImage(outputResource->getImage(renderContext.frameIndex),
                          outputResource->getLayout(renderContext.frameIndex),
                          renderContext.swapChainImages[renderImageIndex],
                          vk::ImageLayout::eTransferDstOptimal, 1, &blitRegion,
                          vk::Filter::eLinear);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[renderImageIndex], 1, 0, 1,
      0, vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageAspectFlagBits::eColor);

  // Handle UI here, abstract later
  imGui->beginFrame();
  profilerUI->render();
  cameraUI->render();
  imGui->endFrame(renderContext.frameIndex);
  imGui->drawFrame(commandBuffer,
                   renderContext.swapChainImageViews[renderImageIndex],
                   renderContext.frameIndex);

  Helper::VulkanHelper::transitionImageLayout(
      commandBuffer, renderContext.swapChainImages[renderImageIndex], 1, 0, 1,
      0, vk::ImageLayout::eColorAttachmentOptimal,
      vk::ImageLayout::ePresentSrcKHR, vk::ImageAspectFlagBits::eColor);

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
      .pSignalSemaphores =
          &renderContext.renderFinishedSemaphores[renderImageIndex]};

  vk::Result submitResult = renderContext.graphicsQueue.submit(
      1, &submitInfo, renderContext.inFlightFences[renderContext.frameIndex]);

  vk::PresentInfoKHR presentInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &renderContext.renderFinishedSemaphores[renderImageIndex],
      .swapchainCount = 1,
      .pSwapchains = &renderContext.swapChain,
      .pImageIndices = &renderImageIndex};

  vk::Result presentResult =
      renderContext.graphicsQueue.presentKHR(presentInfo);

  if (presentResult == vk::Result::eSuboptimalKHR ||
      presentResult == vk::Result::eErrorOutOfDateKHR ||
      renderContext.didFrameBufferSizeChange()) {
    handleSwapchainRecreation();
  } else {
    assert(presentResult == vk::Result::eSuccess);
  }

  renderContext.frameIndex =
      (renderContext.frameIndex + 1) % renderContext.inFlightFrame;
}

void Engine::mainLoop() {
  while (!glfwWindowShouldClose(renderContext.window)) {
    glfwPollEvents();

    int w = 0, h = 0;
    glfwGetFramebufferSize(renderContext.window, &w, &h);
    if (w == 0 || h == 0) {
      glfwWaitEvents();
      continue;
    }

    auto [result, nextRenderImageIndex] = acquireSwapChainImage();
    if (result == vk::Result::eErrorOutOfDateKHR) {
      handleSwapchainRecreation();
      continue;
    }

    renderContext.updateDeltaTime();
    input->update();
    handleInput(renderContext.getDeltaTime());
    camera->update(renderContext.getDeltaTime());
    renderFrame(nextRenderImageIndex);

    input->clearMouseDelta();
  }

  renderContext.device.waitIdle();
}

void Engine::setupExampleRenderGraph() {
  if (renderGraph != nullptr) {
    delete renderGraph;
  }
  renderGraph = new RenderGraph(renderContext);

  GraphResource *colorResource = new GraphResource(
      "color_image", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height, vk::Format::eR16G16B16A16Sfloat,
      vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eColor,
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc |
          vk::ImageUsageFlagBits::eSampled,
      vk::SampleCountFlagBits::e1, renderContext.inFlightFrame, renderContext);
  colorResource->bindSlot(renderContext.bindlessResourceDescriptorSets, 1);

  GraphResource *depthResource = new GraphResource(
      "depth_image", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height, vk::Format::eD32Sfloat,
      vk::ImageLayout::eUndefined, vk::ImageAspectFlagBits::eDepth,
      vk::ImageUsageFlagBits::eDepthStencilAttachment |
          vk::ImageUsageFlagBits::eSampled,
      renderContext.msaaSamples, renderContext.inFlightFrame, renderContext);
  depthResource->bindSlot(renderContext.bindlessResourceDescriptorSets, 2);

  GraphResource *finalColorResource = new GraphResource(
      "final_color", renderContext.swapChainExtent.width,
      renderContext.swapChainExtent.height,
      renderContext.swapChainSurfaceFormat.format, vk::ImageLayout::eUndefined,
      vk::ImageAspectFlagBits::eColor,
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc |
          vk::ImageUsageFlagBits::eSampled,
      renderContext.msaaSamples, renderContext.inFlightFrame, renderContext);
  finalColorResource->bindSlot(renderContext.bindlessResourceDescriptorSets, 3);

  renderGraph->addResource(colorResource);
  renderGraph->addResource(depthResource);
  renderGraph->addResource(finalColorResource);
  renderGraph->setOutputResource(finalColorResource->getName());

  RenderPass::CreateInfo renderingCreateInfo{
      .rendering = {.colorFormats = {colorResource->getFormat()},
                    .depthFormat = depthResource->getFormat()}};
  MainPass *mainPass = new MainPass("main_pass", renderingCreateInfo,
                                    renderContext, *resourceManager);
  mainPass->setColors({{.resource = colorResource,
                        .accessType = RenderPass::ResourceAccessType::Write}});
  mainPass->setDepth({.resource = depthResource,
                      .accessType = RenderPass::ResourceAccessType::Write});

  RenderPass::CreateInfo skyboxCreateInfo{
      .rendering = {.colorFormats = {colorResource->getFormat()},
                    .depthFormat = depthResource->getFormat()}};
  SkyboxPass *skyboxPass = new SkyboxPass("skybox_pass", skyboxCreateInfo,
                                          renderContext, *resourceManager);
  skyboxPass->setColors(
      {{.resource = colorResource,
        .accessType = RenderPass::ResourceAccessType::Modify}});
  skyboxPass->setDepth({.resource = depthResource,
                        .accessType = RenderPass::ResourceAccessType::Read});

  RenderPass::CreateInfo tonemappingCreateInfo{
      .rendering = {.colorFormats = {finalColorResource->getFormat()},
                    .depthFormat = vk::Format::eD32Sfloat}};
  TonemappingPass *tonemappingPass =
      new TonemappingPass("tonemapping_pass", tonemappingCreateInfo,
                          renderContext, *resourceManager);
  tonemappingPass->setSampled(
      {.resource = colorResource,
       .accessType = RenderPass::ResourceAccessType::Read},
      TonemappingPass::SampleSlot::InputImage);
  tonemappingPass->setColors(
      {{.resource = finalColorResource,
        .accessType = RenderPass::ResourceAccessType::Write}});

  renderGraph->addPass(tonemappingPass);
  renderGraph->addPass(mainPass);
  renderGraph->addPass(skyboxPass);

  renderGraph->compile();
}

std::vector<Entity *>
processNodesList(std::vector<RawSceneNode> &nodes,
                 ResourceManager *resourceManager, RenderContext &renderContext,
                 std::unordered_map<std::string, RawTexture> &texturesMap) {
  std::vector<Entity *> entities;

  std::unordered_map<std::string, uint32_t> textureSlots;
  uint32_t nextSlot = 0;

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
    material.setPbrBaseColorFactor(node.pbrProperty.baseColorFactor);

    auto albedoIt = texturesMap.find(node.textureNames[static_cast<size_t>(
        RawSceneNode::TextureIndexer::Albedo)]);
    if (albedoIt != texturesMap.end()) {
      const auto &rawAlbedo = albedoIt->second;
      ResourceHandle<Texture> albedo =
          resourceManager->load<Texture>(rawAlbedo.name, rawAlbedo);

      auto it = textureSlots.find(albedo.getId());
      if (it == textureSlots.end()) {
        nextSlot += 1;
        textureSlots[albedo.getId()] = nextSlot;
      }

      uint32_t slot = textureSlots[albedo.getId()];
      material.setAlbedo({.index = slot, .handle = albedo});
      material.registerAlbedo(renderContext.bindlessDescriptorSets, slot,
                              renderContext);
    }

    auto normalIt = texturesMap.find(node.textureNames[static_cast<size_t>(
        RawSceneNode::TextureIndexer::Normal)]);
    if (normalIt != texturesMap.end()) {
      const auto &rawNormal = normalIt->second;
      ResourceHandle<Texture> normal =
          resourceManager->load<Texture>(rawNormal.name, rawNormal);

      auto it = textureSlots.find(normal.getId());
      if (it == textureSlots.end()) {
        nextSlot += 1;
        textureSlots[normal.getId()] = nextSlot;
      }

      uint32_t slot = textureSlots[normal.getId()];
      material.setNormal({.index = slot, .handle = normal});
      material.registerNormal(renderContext.bindlessDescriptorSets, slot,
                              renderContext);
    }

    auto metallicRoughnessIt =
        texturesMap.find(node.textureNames[static_cast<size_t>(
            RawSceneNode::TextureIndexer::MetallicRoughness)]);
    if (metallicRoughnessIt != texturesMap.end()) {
      const auto &rawMetallicRoughness = metallicRoughnessIt->second;
      ResourceHandle<Texture> normal = resourceManager->load<Texture>(
          rawMetallicRoughness.name, rawMetallicRoughness);

      auto it = textureSlots.find(normal.getId());
      if (it == textureSlots.end()) {
        nextSlot += 1;
        textureSlots[normal.getId()] = nextSlot;
      }

      uint32_t slot = textureSlots[normal.getId()];
      material.setMetallicRoughness({.index = slot, .handle = normal});
      material.registerMetallicRoughness(renderContext.bindlessDescriptorSets,
                                         slot, renderContext);
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

  std::unordered_map<std::string, RawTexture> texturesMap;
  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes,
                                             texturesMap);

  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 0.0f});

  glm::vec3 cameraRotateAxis = {0.0f, 1.0f, 0.0f};
  glm::quat cameraRot = glm::angleAxis(glm::radians(-90.0f), cameraRotateAxis);
  camera->getTransform()->setRotation(cameraRot *
                                      camera->getTransform()->getRotation());

  ubo.directionalLightIntensity = 2.0f;
  ubo.directionalLightDirection = glm::vec3(-0.36f, -0.8f, -0.5f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightIntensity = 36.0f;
  ubo.pointLightPosition = glm::vec3(-7.0f, 4.5f, -0.36f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext, texturesMap);

  return entities;
}

std::vector<Entity *> loadOrientationTest(ResourceManager *resourceManager,
                                          RenderContext &renderContext,
                                          Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/scenes/orientation_test";
  const std::string sceneName = "OrientationTest";

  std::unordered_map<std::string, RawTexture> texturesMap;
  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes,
                                             texturesMap);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 5.0f});

  ubo.directionalLightDirection = glm::vec3(-3.0f, -5.0f, -5.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-3.0f, 5.0f, -3.0f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext, texturesMap);

  return entities;
}

std::vector<Entity *> loadBall(ResourceManager *resourceManager,
                               RenderContext &renderContext, Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/models/baseball_01_4k";
  const std::string sceneName = "baseball_01_4k";

  std::unordered_map<std::string, RawTexture> texturesMap;
  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes,
                                             texturesMap);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 5.0f});

  ubo.directionalLightDirection = glm::vec3(-3.0f, -5.0f, -5.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-3.0f, 5.0f, -3.0f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext, texturesMap);

  for (const auto &e : entities) {
    e->getComponent<TransformComponent>()->setScale({20.0f, 20.0f, 20.0f});
  }

  return entities;
}

std::vector<Entity *> loadChair(ResourceManager *resourceManager,
                                RenderContext &renderContext, Camera *camera) {
  std::vector<RawSceneNode> nodes;

  const std::string scenePath = "resources/models/plastic_monobloc_chair_01_4k";
  const std::string sceneName = "plastic_monobloc_chair_01_4k";

  std::unordered_map<std::string, RawTexture> texturesMap;
  Helper::AssetLoader::loadGltfSceneFromGltf(scenePath, sceneName, nodes,
                                             texturesMap);
  std::cout << "Loaded " << sceneName << ": " << nodes.size() << " nodes.\n";

  camera->getTransform()->setPosition({0.0f, 0.0f, 5.0f});

  ubo.directionalLightDirection = glm::vec3(-3.0f, -5.0f, -5.0f);
  ubo.directionalLightColor = glm::vec3(1.0f, 0.96f, 0.89f);

  ubo.pointLightPosition = glm::vec3(-3.0f, 5.0f, -3.0f);
  ubo.pointLightColor = glm::vec3(1.0f);

  std::vector<Entity *> entities =
      processNodesList(nodes, resourceManager, renderContext, texturesMap);

  for (const auto &e : entities) {
    e->getComponent<TransformComponent>()->setScale({3.0f, 3.0f, 3.0f});
    e->getComponent<TransformComponent>()->setPosition({0.0f, -1.0f, 0.0f});
  }

  return entities;
}

std::vector<Entity *> loadScene(ResourceManager *resourceManager,
                                RenderContext &renderContext, Camera *camera) {

  std::vector<Entity *> entities;
//   entities = loadSponza(resourceManager, renderContext, camera);
  entities = loadBall(resourceManager, renderContext, camera);
  // entities = loadChair(resourceManager, renderContext, camera);
  return entities;
}

void Engine::initRenderObjectsList() {
  skybox = resourceManager->load<Cubemap>("skybox",
                                          "resources/skybox/pizzo_pernice");

  vk::DescriptorImageInfo imageInfo{
      .sampler = skybox->getSampler(),
      .imageView = skybox->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

  vk::WriteDescriptorSet descriptorWrite{
      .dstSet = renderContext.bindlessDescriptorSets,
      .dstBinding = 1,
      .dstArrayElement = 0,
      .descriptorCount = 1,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .pImageInfo = &imageInfo};

  renderContext.device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
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
