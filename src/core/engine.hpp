#pragma once

#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"

namespace SimpleEngine {
namespace Core {
class Engine {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;

  ResourceManager *resourceManager = nullptr;

  void setupExampleRenderGraph();

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
