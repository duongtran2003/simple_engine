#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Mesh : public Resource {
public:
  struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 tangent;
  };

private:
  enum class Source { fromFile, fromMemory };
  Source source;

  std::string path;
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  vk::DeviceSize vertexBufferOffset;
  uint32_t vertexCount = 0;

  vk::Buffer indexBuffer;
  vk::DeviceMemory indexBufferMemory;
  vk::DeviceSize indexBufferOffset;
  uint32_t indexCount = 0;

  void createVertexBuffer(std::vector<Vertex> &v);
  void createIndexBuffer(std::vector<uint32_t> &i);

public:
  Mesh(const std::string &id, const RenderContext &renderContext,
       const std::string &path);
  Mesh(const std::string &id, const RenderContext &renderContext,
       std::vector<Vertex> vertices, std::vector<uint32_t> indices);

  vk::Buffer getVertexBuffer() const;
  vk::Buffer getIndexBuffer() const;

  uint32_t getVertexCount() const;
  uint32_t getIndexCount() const;

  vk::DescriptorSet getTextureDescriptorSet() const;
  void registerTextureToBindlessPool(vk::DescriptorSet bindlessSet,
                                     uint32_t textureSlotIndex);

protected:
  bool doLoad() override;
  void doUnload() override;
};
} // namespace Core
} // namespace SimpleEngine
