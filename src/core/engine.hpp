#pragma once

#include "core/render/render_pass_manager.hpp"
#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
class Engine {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  RenderPassManager *renderPassManager = nullptr;

  ResourceManager *resourceManager = nullptr;

  void setupDeferredRenderer(uint32_t width, uint32_t height);
  void setupHelloTriangleGraph();
  void setupRenderPasses();

  void initWindow();
  void initVulkan();
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  void createSwapChainImageViews();
  void createCommandPool();
  void allocateCommandBuffers();
  void createSyncObjects();

  // For demo only
  void createGraphicsPipeline();

  void mainLoop();
  void renderFrame();

public:
  Engine();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
