#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/render_context.hpp"
#include "core/render_graph/render_graph.hpp"
#include "core/resource/resource_manager.hpp"
#include "vulkan/vulkan.hpp"
#include <chrono>
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Engine {
public:
  struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;

    alignas(16) glm::vec3 lightDirection;
    alignas(16) glm::vec3 objectColor;
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

  void createDescriptorPool();
  void createDescriptorSetLayout();
  void createDescriptorSets();
  void updateUniformBuffer(uint32_t currentFrame, glm::mat4 model);

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
