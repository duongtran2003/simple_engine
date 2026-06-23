#include "render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_hpp_macros.hpp>

namespace SimpleEngine {
namespace Core {

std::vector<const char *> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName};
std::vector<char const *> requiredLayers = {
    // "VK_LAYER_KHRONOS_validation"
};

constexpr uint32_t WIDTH = 800;
constexpr uint32_t HEIGHT = 600;
const std::string APP_NAME = "Simple Engine";
const std::string ENGINE_NAME = "Vulkan";
constexpr uint32_t MAX_FRAME_IN_FLIGHTS = 2;

RenderContext::RenderContext(const RenderContextCreateInfo &createInfo) {
  inFlightFrame = createInfo.inFlightFrame ? createInfo.inFlightFrame
                                           : MAX_FRAME_IN_FLIGHTS;
  height = createInfo.height ? createInfo.height : HEIGHT;
  width = createInfo.width ? createInfo.width : WIDTH;

  initWindow(createInfo);
  createInstance(createInfo);
  createSurface();
  pickPhysicalDevice();
  createDevice();
  createSwapChain();
  createSwapChainImageViews();
  createCommandPool();
  allocateCommandBuffers();
  createSyncObjects();
  createViewport();
  createScissor();
}

void RenderContext::initWindow(const RenderContextCreateInfo &createInfo) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  window = glfwCreateWindow(
      createInfo.width ? static_cast<int>(createInfo.width) : WIDTH,
      createInfo.height ? static_cast<int>(createInfo.height) : HEIGHT,
      !createInfo.appName.empty() ? createInfo.appName.c_str()
                                  : APP_NAME.c_str(),
      nullptr, nullptr);
}

void RenderContext::createInstance(const RenderContextCreateInfo &createInfo) {
  vk::ApplicationInfo appInfo{.pApplicationName =
                                  !createInfo.appName.empty()
                                      ? createInfo.appName.c_str()
                                      : APP_NAME.c_str(),
                              .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                              .pEngineName = ENGINE_NAME.c_str(),
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

  vk::InstanceCreateInfo instanceCreateInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(requiredLayers.size()),
      .ppEnabledLayerNames = requiredLayers.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

  instance = vk::createInstance(instanceCreateInfo, nullptr,
                                vk::detail::defaultDispatchLoaderDynamic);
  vk::detail::defaultDispatchLoaderDynamic.init(instance);
}

void RenderContext::createSurface() {
  VkSurfaceKHR _surface;
  if (glfwCreateWindowSurface(instance, window, nullptr, &_surface) != 0) {
    throw std::runtime_error(
        "Engine::createSurface::ERROR: Failed to create surface.");
  }

  surface = vk::SurfaceKHR(_surface);
}

void RenderContext::pickPhysicalDevice() {
  std::vector<vk::PhysicalDevice> physicalDevices =
      instance.enumeratePhysicalDevices();

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

  physicalDevice = *deviceIt;
}

void RenderContext::createDevice() {
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
      physicalDevice.getQueueFamilyProperties();

  int32_t index = -1;
  for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
    if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
        physicalDevice.getSurfaceSupportKHR(i, surface)) {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index == -1) {
    throw std::runtime_error(
        "Engine::createDevice::ERROR: Couldn't find graphics queue.");
  }
  graphicsQueueFamilyIndex = static_cast<uint32_t>(index);

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

  device = physicalDevice.createDevice(
      deviceCreateInfo, nullptr, vk::detail::defaultDispatchLoaderDynamic);
  vk::detail::defaultDispatchLoaderDynamic.init(device);
  graphicsQueue = device.getQueue(graphicsQueueFamilyIndex, 0);
}

void RenderContext::createSwapChain() {
  vk::SurfaceCapabilitiesKHR surfaceCapabilities =
      physicalDevice.getSurfaceCapabilitiesKHR(surface);

  vk::Extent2D _swapChainExtent;
  if (surfaceCapabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    _swapChainExtent = surfaceCapabilities.currentExtent;
  } else {
    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    uint32_t clampedW = std::clamp<uint32_t>(
        static_cast<uint32_t>(w), surfaceCapabilities.minImageExtent.width,
        surfaceCapabilities.maxImageExtent.width);
    uint32_t clampedH = std::clamp<uint32_t>(
        static_cast<uint32_t>(h), surfaceCapabilities.minImageExtent.height,
        surfaceCapabilities.maxImageExtent.height);
    _swapChainExtent = vk::Extent2D({.width = clampedW, .height = clampedH});
  }
  swapChainExtent = _swapChainExtent;

  uint32_t minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
  if ((0 < surfaceCapabilities.maxImageCount) &&
      (surfaceCapabilities.maxImageCount < minImageCount)) {
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  std::vector<vk::SurfaceFormatKHR> availableFormats =
      physicalDevice.getSurfaceFormatsKHR(surface);
  vk::SurfaceFormatKHR _swapChainSurfaceFormat;
  const auto formatIt = std::ranges::find_if(
      availableFormats, [](const vk::SurfaceFormatKHR &format) {
        return format.format == vk::Format::eR16G16B16A16Sfloat &&
               format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
      });

  if (formatIt != availableFormats.end()) {
    _swapChainSurfaceFormat = *formatIt;
  } else {
    _swapChainSurfaceFormat = availableFormats[0];
  }
  swapChainSurfaceFormat = _swapChainSurfaceFormat;

  std::vector<vk::PresentModeKHR> availablePresentModes =
      physicalDevice.getSurfacePresentModesKHR(surface);
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
      .surface = surface,
      .minImageCount = minImageCount,
      .imageFormat = _swapChainSurfaceFormat.format,
      .imageColorSpace = _swapChainSurfaceFormat.colorSpace,
      .imageExtent = _swapChainExtent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment |
                    vk::ImageUsageFlagBits::eTransferDst,
      .imageSharingMode = vk::SharingMode::eExclusive,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = presentMode,
      .clipped = true};

  swapChain = device.createSwapchainKHR(swapChainCreateInfo);
  swapChainImages = device.getSwapchainImagesKHR(swapChain);
}

void RenderContext::createSwapChainImageViews() {
  for (const auto &imageView : swapChainImageViews) {
    device.destroyImageView(imageView);
  }
  swapChainImageViews.clear();

  vk::ImageViewCreateInfo createInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapChainSurfaceFormat.format,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};
  for (const auto &image : swapChainImages) {
    createInfo.image = image;
    vk::ImageView imageView = device.createImageView(createInfo);
    swapChainImageViews.push_back(imageView);
  }
}

void RenderContext::createCommandPool() {
  vk::CommandPoolCreateInfo createInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = graphicsQueueFamilyIndex};

  commandPool = device.createCommandPool(createInfo);
}

void RenderContext::allocateCommandBuffers() {
  if (!commandBuffers.empty()) {
    device.freeCommandBuffers(commandPool, commandBuffers);
  }

  vk::CommandBufferAllocateInfo allocateInfo{
      .commandPool = commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = inFlightFrame};

  commandBuffers = device.allocateCommandBuffers(allocateInfo);
}

void RenderContext::createSyncObjects() {
  for (const auto &semaphore : presentCompleteSemaphores) {
    device.destroySemaphore(semaphore);
  }
  for (const auto &semaphore : renderFinishedSemaphores) {
    device.destroySemaphore(semaphore);
  }

  presentCompleteSemaphores.clear();
  renderFinishedSemaphores.clear();

  for (size_t i = 0; i < swapChainImages.size(); i++) {
    vk::Semaphore semaphore = device.createSemaphore({});
    renderFinishedSemaphores.emplace_back(semaphore);
  }

  for (size_t i = 0; i < inFlightFrame; i++) {
    vk::Semaphore semaphore = device.createSemaphore({});
    presentCompleteSemaphores.emplace_back(semaphore);

    vk::FenceCreateInfo fenceCreateInfo{.flags =
                                            vk::FenceCreateFlagBits::eSignaled};
    vk::Fence fence = device.createFence(fenceCreateInfo);
    inFlightFences.emplace_back(fence);
  }
}

void RenderContext::createViewport() {
  vk::Viewport _viewport{.x = 0.0f,
                         .y = 0.0f,
                         .width = static_cast<float>(swapChainExtent.width),
                         .height = static_cast<float>(swapChainExtent.height),
                         .minDepth = 0.0f,
                         .maxDepth = 1.0f};
  viewport = _viewport;
}

void RenderContext::createScissor() {
  vk::Rect2D _scissor{.offset = {.x = 0, .y = 0}, .extent = swapChainExtent};
  scissor = _scissor;
}
} // namespace Core
} // namespace SimpleEngine
