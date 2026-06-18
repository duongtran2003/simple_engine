#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/engine.hpp"
#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/shader.hpp"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

#include <GLFW/glfw3.h>
#include <array>
#include <cstdint>
#include <iostream>
#include <vulkan/vulkan_hpp_macros.hpp>

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
constexpr uint32_t MAX_FRAME_IN_FLIGHTS = 2;

std::vector<const char *> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName};
std::vector<char const *> requiredLayers = {"VK_LAYER_KHRONOS_validation"};

namespace SimpleEngine {
namespace Core {

Engine::Engine() {
  renderContext = RenderContext();
  renderContext.inFlightFrame = MAX_FRAME_IN_FLIGHTS;
  resourceManager = new ResourceManager(renderContext);

  initWindow();
  initVulkan();

  renderGraph = new RenderGraph(renderContext);
}

void Engine::initWindow() {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  renderContext.window =
      glfwCreateWindow(WIDTH, HEIGHT, "Engine", nullptr, nullptr);
}

void Engine::createInstance() {
  vk::ApplicationInfo appInfo{.pApplicationName = "Simple Engine",
                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                              .pEngineName = "Vulkan",
                              .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                              .apiVersion = vk::ApiVersion14};

  auto layerProperties = vk::enumerateInstanceLayerProperties();
  auto unsupportedLayerIt = std::ranges::find_if(
      requiredLayers, [&layerProperties](auto const &requiredLayer) {
        return std::ranges::none_of(
            layerProperties, [requiredLayer](auto const &layerProperty) {
              return strcmp(layerProperty.layerName, requiredLayer) == 0;
            });
      });
  if (unsupportedLayerIt != requiredLayers.end()) {
    throw std::runtime_error("Engine::initInstance::ERROR: Unsupported layer.");
  }

  uint32_t glfwExtensionCount = 0;
  auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  std::vector<char const *> requiredExtensions(
      glfwExtensions, glfwExtensions + glfwExtensionCount);
  auto extensionProperties = vk::enumerateInstanceExtensionProperties();
  auto unsupportedPropertyIt = std::ranges::find_if(
      requiredExtensions,
      [&extensionProperties](auto const &requiredExtension) {
        return std::ranges::none_of(
            extensionProperties,
            [requiredExtension](auto const &extensionProperty) {
              return strcmp(extensionProperty.extensionName,
                            requiredExtension) == 0;
            });
      });
  if (unsupportedPropertyIt != requiredExtensions.end()) {
    throw std::runtime_error(
        "Engine::initInstance::ERROR: Unsupported extension.");
  }

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

  renderContext.instance = vk::createInstance(
      createInfo, nullptr, vk::detail::defaultDispatchLoaderDynamic);
  vk::detail::defaultDispatchLoaderDynamic.init(renderContext.instance);
}

void Engine::createSurface() {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(renderContext.instance, renderContext.window,
                              nullptr, &surface) != 0) {
    throw std::runtime_error(
        "Engine::createSurface::ERROR: Failed to create surface.");
  }

  renderContext.surface = vk::SurfaceKHR(surface);
}

void Engine::pickPhysicalDevice() {
  std::vector<vk::PhysicalDevice> physicalDevices =
      renderContext.instance.enumeratePhysicalDevices();

  auto isDeviceSuitable = [](vk::PhysicalDevice &device) {
    bool supportsVulkan1_3 =
        device.getProperties().apiVersion >= vk::ApiVersion13;
    auto queueFamilies = device.getQueueFamilyProperties();
    bool supportsGraphics =
        std::ranges::any_of(queueFamilies, [](auto const &queueFamilyProperty) {
          return !!(queueFamilyProperty.queueFlags &
                    vk::QueueFlagBits::eGraphics);
        });

    auto deviceExtensions = device.enumerateDeviceExtensionProperties();
    bool supportsRequiredExtensions = std::ranges::all_of(
        requiredDeviceExtensions,
        [&deviceExtensions](auto const &requiredDeviceExtension) {
          return std::ranges::any_of(
              deviceExtensions,
              [requiredDeviceExtension](auto const &deviceExtension) {
                return strcmp(deviceExtension.extensionName,
                              requiredDeviceExtension) == 0;
              });
        });

    auto features = device.template getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    bool supportsRequiredFeatures =
        features.template get<vk::PhysicalDeviceFeatures2>()
            .features.samplerAnisotropy &&
        features.template get<vk::PhysicalDeviceVulkan11Features>()
            .shaderDrawParameters &&
        features.template get<vk::PhysicalDeviceVulkan13Features>()
            .dynamicRendering &&
        features
            .template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
            .extendedDynamicState;

    return supportsVulkan1_3 && supportsGraphics &&
           supportsRequiredExtensions && supportsRequiredFeatures;
  };

  auto const deviceIt = std::ranges::find_if(
      physicalDevices, [&](vk::PhysicalDevice &physicalDevice) {
        return isDeviceSuitable(physicalDevice);
      });

  if (deviceIt == physicalDevices.end()) {
    throw std::runtime_error(
        "Engine::pickPhysicalDevice::ERROR: Failed to find physical device.");
  }

  renderContext.physicalDevice = *deviceIt;
}

void Engine::createDevice() {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      renderContext.physicalDevice.getQueueFamilyProperties();

  int32_t index = -1;
  for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
    if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
        renderContext.physicalDevice.getSurfaceSupportKHR(
            i, renderContext.surface)) {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index == -1) {
    throw std::runtime_error(
        "Engine::createDevice::ERROR: Couldn't find graphics queue.");
  }
  renderContext.graphicsQueueFamilyIndex = static_cast<uint32_t>(index);

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featuresChain = {
          {.features = {.sampleRateShading = true, .samplerAnisotropy = true}},
          {.shaderDrawParameters = true},
          {.synchronization2 = true, .dynamicRendering = true},
          {.extendedDynamicState = true}};

  float queuePriority = 0.5f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
      .queueFamilyIndex = static_cast<uint32_t>(index),
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  vk::DeviceCreateInfo deviceCreateInfo{
      .pNext = &featuresChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &deviceQueueCreateInfo,
      .enabledExtensionCount =
          static_cast<uint32_t>(requiredDeviceExtensions.size()),
      .ppEnabledExtensionNames = requiredDeviceExtensions.data()};

  renderContext.device = renderContext.physicalDevice.createDevice(
      deviceCreateInfo, nullptr, vk::detail::defaultDispatchLoaderDynamic);
  vk::detail::defaultDispatchLoaderDynamic.init(renderContext.device);
  renderContext.graphicsQueue =
      renderContext.device.getQueue(renderContext.graphicsQueueFamilyIndex, 0);
}

void Engine::createSwapChain() {
  vk::SurfaceCapabilitiesKHR surfaceCapabilities =
      renderContext.physicalDevice.getSurfaceCapabilitiesKHR(
          renderContext.surface);

  vk::Extent2D swapChainExtent;
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    swapChainExtent = surfaceCapabilities.currentExtent;
  } else {
    int w, h;
    glfwGetFramebufferSize(renderContext.window, &w, &h);
    uint32_t clampedW = std::clamp<uint32_t>(
        static_cast<uint32_t>(w), surfaceCapabilities.minImageExtent.width,
        surfaceCapabilities.maxImageExtent.width);
    uint32_t clampedH = std::clamp<uint32_t>(
        static_cast<uint32_t>(h), surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.height);
    swapChainExtent = vk::Extent2D({.width = clampedW, .height = clampedH});
  }
  renderContext.swapChainExtent = swapChainExtent;

  uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  std::vector<vk::SurfaceFormatKHR> availableFormats =
      renderContext.physicalDevice.getSurfaceFormatsKHR(renderContext.surface);
  vk::SurfaceFormatKHR swapChainSurfaceFormat;
  const auto formatIt = std::ranges::find_if(
      availableFormats, [](const vk::SurfaceFormatKHR &format) {
        return format.format == vk::Format::eB8G8R8A8Srgb &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
      });

  if (formatIt != availableFormats.end()) {
    swapChainSurfaceFormat = *formatIt;
  } else {
    swapChainSurfaceFormat = availableFormats[0];
  }
  renderContext.swapChainSurfaceFormat = swapChainSurfaceFormat;

  std::vector<vk::PresentModeKHR> availablePresentModes =
      renderContext.physicalDevice.getSurfacePresentModesKHR(
          renderContext.surface);
  bool hasMailbox = false;
  for (const auto &mode : availablePresentModes) {
    if (mode == vk::PresentModeKHR::eMailbox) {
      hasMailbox = true;
      break;
    }
  }

  vk::PresentModeKHR presentMode =
      hasMailbox ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;

  vk::SwapchainCreateInfoKHR swapChainCreateInfo{
      .surface = renderContext.surface,
      .minImageCount = minImageCount,
      .imageFormat = renderContext.swapChainSurfaceFormat.format,
      .imageColorSpace = renderContext.swapChainSurfaceFormat.colorSpace,
      .imageExtent = renderContext.swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true};

  renderContext.swapChain =
      renderContext.device.createSwapchainKHR(swapChainCreateInfo);
  renderContext.swapChainImages =
      renderContext.device.getSwapchainImagesKHR(renderContext.swapChain);
}

void Engine::createSwapChainImageViews() {
  for (const auto &imageView : renderContext.swapChainImageViews) {
    renderContext.device.destroyImageView(imageView);
  }
  renderContext.swapChainImageViews.clear();

  vk::ImageViewCreateInfo createInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = renderContext.swapChainSurfaceFormat.format,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  for (const auto &image : renderContext.swapChainImages) {
    createInfo.image = image;
    vk::ImageView imageView = renderContext.device.createImageView(createInfo);
    renderContext.swapChainImageViews.push_back(imageView);
  }
}

void Engine::createCommandPool() {
  vk::CommandPoolCreateInfo createInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = renderContext.graphicsQueueFamilyIndex};

  renderContext.commandPool =
      renderContext.device.createCommandPool(createInfo);
}

void Engine::allocateCommandBuffers() {
  if (!renderContext.commandBuffers.empty()) {
    renderContext.device.freeCommandBuffers(renderContext.commandPool,
                                            renderContext.commandBuffers);
  }

  vk::CommandBufferAllocateInfo allocateInfo{
      .commandPool = renderContext.commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = renderContext.inFlightFrame};

  renderContext.commandBuffers =
      renderContext.device.allocateCommandBuffers(allocateInfo);
}

void Engine::createSyncObjects() {
  for (const auto &semaphore : renderContext.presentCompleteSemaphores) {
    renderContext.device.destroySemaphore(semaphore);
  }
  for (const auto &semaphore : renderContext.renderFinishedSemaphores) {
    renderContext.device.destroySemaphore(semaphore);
  }

  renderContext.presentCompleteSemaphores.clear();
  renderContext.renderFinishedSemaphores.clear();

  for (size_t i = 0; i < renderContext.swapChainImages.size(); i++) {
    vk::Semaphore semaphore = renderContext.device.createSemaphore({});
    renderContext.renderFinishedSemaphores.emplace_back(semaphore);
  }

  for (size_t i = 0; i < renderContext.inFlightFrame; i++) {
    vk::Semaphore semaphore = renderContext.device.createSemaphore({});
    renderContext.presentCompleteSemaphores.emplace_back(semaphore);

    vk::FenceCreateInfo fenceCreateInfo{.flags =
                                            vk::FenceCreateFlagBits::eSignaled};
    vk::Fence fence = renderContext.device.createFence(fenceCreateInfo);
    renderContext.inFlightFences.emplace_back(fence);
  }
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

  vk::Viewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(renderContext.swapChainExtent.width),
      .height = static_cast<float>(renderContext.swapChainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  renderContext.viewport = viewport;

  vk::Rect2D scissor{.offset = {.x = 0, .y = 0},
                     .extent = renderContext.swapChainExtent};
  renderContext.scissor = scissor;

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport,
                                                 vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState{
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates = dynamicStates.data()};

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .pViewports = &viewport,
                                                    .scissorCount = 1,
                                                    .pScissors = &scissor};

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
      .pColorBlendState = &colorBlending,
      .pDynamicState = &dynamicState,
      .layout = pipelineLayout,
      .renderPass = nullptr};

  vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo{
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &renderContext.swapChainSurfaceFormat.format};

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

void Engine::initVulkan() {
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createDevice();
  createSwapChain();
  createSwapChainImageViews();
  createCommandPool();
  allocateCommandBuffers();
  createSyncObjects();
  createGraphicsPipeline();
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
  renderGraph->execute(renderContext.frameIndex);

  vk::CommandBuffer commandBuffer =
      renderContext.commandBuffers[renderContext.frameIndex];
  commandBuffer.reset();
  vk::CommandBufferBeginInfo beginInfo{
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
  commandBuffer.begin(beginInfo);

  vk::ImageMemoryBarrier copyTarget{
      .srcAccessMask = vk::AccessFlagBits::eNone,
      .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
      .oldLayout = vk::ImageLayout::eUndefined,
      .newLayout = vk::ImageLayout::eTransferDstOptimal,
      .image = renderContext.swapChainImages[imageIndex],
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits::eByRegion,
      0, nullptr, 0, nullptr, 1, &copyTarget);

  vk::ImageCopy copyRegion{
      .srcSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .dstSubresource = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                         .baseArrayLayer = 0,
                         .layerCount = 1},
      .extent = vk::Extent3D(renderContext.swapChainExtent.width,
                             renderContext.swapChainExtent.height, 1)};

  commandBuffer.copyImage(renderGraph->getResource("FinalColor")->image,
                          vk::ImageLayout::eTransferSrcOptimal,
                          renderContext.swapChainImages[imageIndex],
                          vk::ImageLayout::eTransferDstOptimal, 1, &copyRegion);

  vk::ImageMemoryBarrier presentTarget{
      .srcAccessMask = vk::AccessFlagBits::eTransferWrite,
      .dstAccessMask = vk::AccessFlagBits::eNone,
      .oldLayout = vk::ImageLayout::eTransferDstOptimal,
      .newLayout = vk::ImageLayout::ePresentSrcKHR,
      .image = renderContext.swapChainImages[imageIndex],
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eBottomOfPipe,
                                vk::DependencyFlagBits::eByRegion, 0, nullptr,
                                0, nullptr, 1, &presentTarget);

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
      .pSignalSemaphores =
          &renderContext.renderFinishedSemaphores[renderContext.frameIndex]};

  vk::Result submitResult = renderContext.graphicsQueue.submit(
      1, &submitInfo, renderContext.inFlightFences[renderContext.frameIndex]);

  vk::PresentInfoKHR presentInfoKHR{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores =
          &renderContext.renderFinishedSemaphores[renderContext.frameIndex],
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
    renderFrame();
  }

  renderContext.device.waitIdle();
}

void Engine::setupHelloTriangleGraph() {
  std::cout << "Beginning setting up hello triangle graph\n";
  renderGraph->addResource(
      "FinalColor", renderContext.swapChainSurfaceFormat.format,
      {.width = WIDTH, .height = HEIGHT},
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);
  renderGraph->addPass(
      "RenderPass", {}, {"FinalColor"},
      [&](vk::CommandBuffer &commandBuffer, RenderGraph &renderGraph) {
        std::array<vk::RenderingAttachmentInfoKHR, 3> colorAttachments;
        vk::RenderingAttachmentInfoKHR colorAttachment{
            .imageView = renderGraph.getResource("FinalColor")->view,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore};

        vk::RenderingInfoKHR renderingInfo{
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = {.width = WIDTH, .height = HEIGHT}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment,
        };

        commandBuffer.beginRendering(renderingInfo);
        // Bind stuff and render stuff here
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                                   renderContext.graphicsPipeline);
        commandBuffer.setViewport(0, renderContext.viewport);
        commandBuffer.setScissor(0, renderContext.scissor);
        commandBuffer.draw(3, 1, 0, 0);

        commandBuffer.endRendering();
      });
}

void Engine::setupDeferredRenderer(uint32_t w, uint32_t h) {
  std::cout << "Setting up deferred renderer\n";

  renderGraph->addResource("GBuffer_Position", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("GBuffer_Normal", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("GBuffer_Albedo", vk::Format::eR16G16B16A16Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eColorAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eShaderReadOnlyOptimal);

  renderGraph->addResource("Depth", vk::Format::eD32Sfloat,
                           {.width = w, .height = h},
                           vk::ImageUsageFlagBits::eDepthStencilAttachment |
                               vk::ImageUsageFlagBits::eInputAttachment,
                           vk::ImageLayout::eUndefined,
                           vk::ImageLayout::eDepthStencilAttachmentOptimal);

  renderGraph->addResource(
      "FinalColor", vk::Format::eR8G8B8A8Unorm, {.width = w, .height = h},
      vk::ImageUsageFlagBits::eColorAttachment |
          vk::ImageUsageFlagBits::eTransferSrc,
      vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal);

  renderGraph->addPass(
      "GeometryPass", {},
      {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo", "Depth"},
      [&](vk::CommandBuffer &commandBuffer, RenderGraph &renderGraph) {
        std::array<vk::RenderingAttachmentInfoKHR, 3> colorAttachments;

        colorAttachments[0]
            .setImageView(renderGraph.getResource("GBuffer_Position")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        colorAttachments[1]
            .setImageView(renderGraph.getResource("GBuffer_Normal")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        colorAttachments[2]
            .setImageView(renderGraph.getResource("GBuffer_Albedo")->view)
            .setImageLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore);

        vk::RenderingAttachmentInfoKHR depthAttachment{
            .imageView = renderGraph.getResource("Depth")->view,
            .imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearDepthStencilValue(1.0f, 0)};

        vk::RenderingInfoKHR renderingInfo{
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = {.width = w, .height = h}},
            .layerCount = 1,
            .colorAttachmentCount = colorAttachments.size(),
            .pColorAttachments = colorAttachments.data(),
            .pDepthAttachment = &depthAttachment,
        };

        commandBuffer.beginRendering(renderingInfo);
        // Bind stuff and render stuff here
        commandBuffer.endRendering();
      });

  renderGraph->addPass(
      "LightingPass",
      {"GBuffer_Position", "GBuffer_Normal", "GBuffer_Albedo", "Depth"},
      {"FinalColor"},
      [&](vk::CommandBuffer &commandBuffer, RenderGraph &renderGraph) {
        vk::RenderingAttachmentInfoKHR colorAttachment{
            .imageView = renderGraph.getResource("FinalColor")->view,
            .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
            .loadOp = vk::AttachmentLoadOp::eClear,
            .storeOp = vk::AttachmentStoreOp::eStore,
            .clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)};

        vk::RenderingInfoKHR renderingInfo{
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = {.width = w, .height = h}},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachment};

        commandBuffer.beginRendering(renderingInfo);
        // Bind stuff and render stuff here
        commandBuffer.endRendering();
      });
  renderGraph->compile();
}

void Engine::run() {
  std::cout << "App run\n";
  setupHelloTriangleGraph();
  renderGraph->compile();
  mainLoop();
}
} // namespace Core
} // namespace SimpleEngine
