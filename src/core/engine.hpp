#pragma once

#include "core/camera.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include <chrono>

namespace SimpleEngine {
namespace Core {
class Engine {
private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  ResourceManager *resourceManager = nullptr;
  Input *input = nullptr;
  Camera *camera = nullptr;

  std::chrono::high_resolution_clock::time_point lastFrameTime;
  float deltaTime = 0.0f;

  void setupExampleRenderGraph();

  // For demo only
  void createGraphicsPipeline();

  void mainLoop();
  void renderFrame();

  void handleInput();
  void updateFrameTime();

public:
  Engine();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
