#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/resource_manager.hpp"
#include "core/resource/texture.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <imgui.h>
#include <vector>

namespace SimpleEngine {
namespace UI {
class ImGuiVulkan {
public:
  struct ImGuiVkBuffer {
    vk::Buffer buffer;
    vk::DeviceMemory memory;
    void *mapped;
  };

  struct ImGuiVulkanPushConstant {
    glm::vec2 scale;
    glm::vec2 translate;
  };

private:
  const Core::RenderContext &context;
  Core::ResourceManager &resourceManager;

  std::vector<ImGuiVkBuffer> vertexBuffers;
  std::vector<ImGuiVkBuffer> indexBuffers;

  uint32_t vertexCount = 0;
  uint32_t indexCount = 0;

  Core::ResourceHandle<Core::Texture> fontTexture;

  vk::PipelineCache pipelineCache;
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline pipeline;

  vk::DescriptorPool descriptorPool;
  vk::DescriptorSetLayout descriptorSetLayout;
  vk::DescriptorSet descriptorSet;

  ImGuiStyle uiStyle;

  bool needUpdateBuffers = true;
  vk::PipelineRenderingCreateInfo renderingInfo{};
  vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;

  void createVertexBuffers(vk::DeviceSize bufferSize);
  void createVertexBuffer(vk::DeviceSize bufferSize, uint32_t frameIndex);
  void clearVertexBuffers();
  void clearVertexBuffer(uint32_t frameIndex);

  void createIndexBuffers(vk::DeviceSize bufferSize);
  void createIndexBuffer(vk::DeviceSize bufferSize, uint32_t frameIndex);
  void clearIndexBuffers();
  void clearIndexBuffer(uint32_t frameIndex);

public:
  ImGuiVulkan(const Core::RenderContext &context,
              Core::ResourceManager &resourceManager);
  ~ImGuiVulkan();

  void init(float w, float h);
  void initResources();
  void setStyle(uint32_t index);
  void updateTexture(ImTextureData *tex);

  bool newFrame();
  void updateBuffers(uint32_t frameIndex);
  void drawFrame(vk::CommandBuffer &commandBuffer);

  void handleKey(int key, int scancode, int action, int mode);
  void handleMousePos(float x, float y);
  void handleMouseButton(int button, bool pressed);
  bool getWantKeyCapture();

  void charPressed(uint32_t key);
};
} // namespace UI
} // namespace SimpleEngine
