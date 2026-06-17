#pragma once

#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include <cstdint>

namespace SimpleEngine {
namespace Core {
class Engine {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;

  void setupDeferredRenderer(uint32_t width, uint32_t height);

  void initWindow();
  void initVulkan();
  void createInstance();
  void createSurface();
  void pickPhysicalDevice();
  void createDevice();
  void createSwapChain();
  void createSwapChainImageViews();
  void createCommandPool();

  void renderFrame();

public:
  Engine();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
