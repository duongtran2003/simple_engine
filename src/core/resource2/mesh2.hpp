#pragma once

#include "core/buffer/index_buffer.hpp"
#include "core/buffer/vertex_buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <memory>
#include <string>

namespace SimpleEngine {
namespace Core {

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;

  static vk::VertexInputBindingDescription GetBindingDescription() {
    return vk::VertexInputBindingDescription{.binding = 0,
                                             .stride = sizeof(Vertex),
                                             .inputRate =
                                                 vk::VertexInputRate::eVertex};
  }

  static std::array<vk::VertexInputAttributeDescription, 3>
  GetAttributeDescriptions() {
    return std::array<vk::VertexInputAttributeDescription, 3>{
        vk::VertexInputAttributeDescription{
            .location = 0,
            .binding = 0,
            .format = vk::Format::eR32G32B32Sfloat,
            .offset = offsetof(Vertex, Position)},
        vk::VertexInputAttributeDescription{.location = 1,
                                            .binding = 0,
                                            .format =
                                                vk::Format::eR32G32B32Sfloat,
                                            .offset = offsetof(Vertex, Normal)},
        vk::VertexInputAttributeDescription{.location = 2,
                                            .binding = 0,
                                            .format = vk::Format::eR32G32Sfloat,
                                            .offset =
                                                offsetof(Vertex, TexCoords)}};
  }
};

class Mesh2 {
public:
  Mesh2() = default;

  Mesh2(Mesh2 &&) noexcept = default;
  Mesh2 &operator=(Mesh2 &&) noexcept = default;

  Mesh2(const Mesh2 &) = delete;
  Mesh2 &operator=(const Mesh2 &) = delete;

  Mesh2(const std::string &identifier,
        std::unique_ptr<VertexBuffer> vertexBuffer,
        std::unique_ptr<IndexBuffer> indexBuffer,
        vk::IndexType indexType = vk::IndexType::eUint32);
  ~Mesh2();

  const std::string &GetIdentifier() const;
  const VertexBuffer *GetVertexBuffer() const;
  const IndexBuffer *GetIndexBuffer() const;
  uint32_t GetVertexCount() const;
  uint32_t GetIndexCount() const;
  vk::IndexType GetIndexType() const;

private:
  std::string m_Identifier;
  std::unique_ptr<VertexBuffer> m_VertexBuffer = nullptr;
  std::unique_ptr<IndexBuffer> m_IndexBuffer = nullptr;
};
} // namespace Core
} // namespace SimpleEngine
