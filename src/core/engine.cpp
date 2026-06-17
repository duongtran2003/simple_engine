#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "core/engine.hpp"
#include "core/render_context.hpp"
#include "core/render_graph.hpp"
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

std::vector<const char *> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName};
std::vector<char const *> requiredLayers = {"VK_LAYER_KHRONOS_validation"};

namespace SimpleEngine {
namespace Core {

Engine::Engine() {
  renderContext = RenderContext();
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
    swapChainExtent = {
        std::clamp<uint32_t>(w, surfaceCapabilities.minImageExtent.width,
                             surfaceCapabilities.maxImageExtent.width),
        std::clamp<uint32_t>(h, surfaceCapabilities.minImageExtent.height,
                             surfaceCapabilities.maxImageExtent.height)};
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

void Engine::initVulkan() {
  createInstance();
  createSurface();
  pickPhysicalDevice();
  createDevice();
  createSwapChain();
  createSwapChainImageViews();
  createCommandPool();
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
  // renderGraph->compile();
}

void Engine::run() {
  std::cout << "App run\n";
  setupDeferredRenderer(800, 600);
}
} // namespace Core
} // namespace SimpleEngine
