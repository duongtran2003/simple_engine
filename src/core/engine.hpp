#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <chrono>
#include <glm/ext/matrix_float4x4.hpp>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Engine {
public:
  struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

  struct UboBuffer {
    vk::Buffer buffer{nullptr};
    vk::DeviceMemory memory{nullptr};
    void *mapped = nullptr;
  };

private:
  RenderContext renderContext;
  RenderGraph *renderGraph = nullptr;
  ResourceManager *resourceManager = nullptr;
  Input *input = nullptr;
  Camera *camera = nullptr;

  std::vector<Entity *> renderObjects;
  std::vector<UboBuffer> uniformBuffers;

  std::chrono::high_resolution_clock::time_point lastFrameTime;
  float deltaTime = 0.0f;

  void createUniformBuffers();

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
