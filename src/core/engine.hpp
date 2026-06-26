#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include <chrono>
#include <cstdint>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Engine {
public:
  struct PushConstants {
    glm::mat4 modelMatrix;
    glm::vec3 cameraPos;
    uint32_t albedoIndex;
    uint32_t normalIndex;
    bool useNormalMap;
  };

private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  ResourceManager *resourceManager = nullptr;
  Input *input = nullptr;
  Camera *camera = nullptr;

  std::vector<Entity *> renderObjects;

  std::chrono::high_resolution_clock::time_point lastFrameTime;
  float deltaTime = 0.0f;

  bool useNormalMap = false;

  void setupExampleRenderGraph();
  void initRenderObjectsList();

  // For demo only
  void createGraphicsPipeline();

  void mainLoop();
  void renderFrame();

  void handleInput(float delta);
  void updateFrameTime();

public:
  Engine();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
