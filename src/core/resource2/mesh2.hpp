#pragma once

#include "core/buffer/buffer.hpp"
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

  Mesh2(const std::string &identifier, std::unique_ptr<Buffer> vertexBuffer,
       std::unique_ptr<Buffer> indexBuffer, uint32_t vertexCount,
       uint32_t indexCount, vk::IndexType indexType = vk::IndexType::eUint32);
  ~Mesh2();

  const std::string &GetIdentifier() const;
  const Buffer *GetVertexBuffer() const;
  const Buffer *GetIndexBuffer() const;
  uint32_t GetVertexCount() const;
  uint32_t GetIndexCount() const;
  vk::IndexType GetIndexType() const;

private:
  std::string m_Identifier;
  std::unique_ptr<Buffer> m_VertexBuffer = nullptr;
  std::unique_ptr<Buffer> m_IndexBuffer = nullptr;
  uint32_t m_VertexCount = 0;
  uint32_t m_IndexCount = 0;
  vk::IndexType m_IndexType = vk::IndexType::eUint32;
};
} // namespace Core
} // namespace SimpleEngine
