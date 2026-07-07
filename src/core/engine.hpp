#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/scene/scene.hpp"
#include "core/system/culling_system.hpp"
#include <chrono>
#include <cstdint>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Engine {
public:
  struct PushConstants {
    alignas(16) glm::mat4 modelMatrix;
    alignas(16) glm::vec3 cameraPos;
    alignas(16) glm::vec4 baseCol;
    alignas(16) uint32_t albedoIndex;
    alignas(16) uint32_t normalIndex;
    alignas(16) uint32_t useNormalMap;
  };

private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  ResourceManager *resourceManager = nullptr;
  Input *input = nullptr;
  Camera *camera = nullptr;
  CullingSystem *cullingSystem = nullptr;
  Scene *scene = nullptr;

  std::vector<Entity *> renderObjects;

  std::chrono::high_resolution_clock::time_point lastFrameTime;
  float deltaTime = 0.0f;

  bool useNormalMap = true;

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
