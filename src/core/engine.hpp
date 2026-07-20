#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/cubemap.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/system/culling_system.hpp"
#include "ui/camera_ui.hpp"
#include "ui/imgui_vulkan.hpp"
#include <chrono>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Engine {
public:
  struct ObjectData {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 normalModel;
    alignas(16) glm::vec4 pbrBaseColorFactor;
    alignas(4) float pbrMetallicFactor;
    alignas(4) float pbrRoughnessFactor;
  };

private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  ResourceManager *resourceManager = nullptr;
  Input *input = nullptr;
  Camera *camera = nullptr;
  CullingSystem *cullingSystem = nullptr;

  ResourceHandle<Cubemap> skybox;

  // UI stuff
  UI::ImGuiVulkan *imGui;
  UI::CameraUI *cameraUI;

  std::vector<Entity *> renderObjects;

  std::chrono::high_resolution_clock::time_point lastFrameTime;
  float deltaTime = 0.0f;

  void setupExampleRenderGraph();
  void initRenderObjectsList();

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
