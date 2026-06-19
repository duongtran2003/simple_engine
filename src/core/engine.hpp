#pragma once

#include "core/render/render_pass_manager.hpp"
#include "core/render_context.hpp"
#include "core/render_graph.hpp"
#include "core/resource/resource_manager.hpp"

namespace SimpleEngine {
namespace Core {
class Engine {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  RenderPassManager *renderPassManager = nullptr;

  ResourceManager *resourceManager = nullptr;

  void setupRenderPasses();

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
