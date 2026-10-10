#include "core/resource2/mesh2.hpp"
#include "core/buffer/buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SimpleEngine {
namespace Core {
Mesh2::Mesh2(const std::string &identifier, std::unique_ptr<Buffer> vertexBuffer,
           std::unique_ptr<Buffer> indexBuffer, uint32_t vertexCount,
           uint32_t indexCount, vk::IndexType indexType) {
  m_Identifier = identifier;
  m_VertexBuffer = std::move(vertexBuffer);
  m_IndexBuffer = std::move(indexBuffer);
  m_VertexCount = vertexCount;
  m_IndexCount = indexCount;
  m_IndexType = indexType;
}

Mesh2::~Mesh2() {}

const std::string &Mesh2::GetIdentifier() const { return m_Identifier; }
const Buffer *Mesh2::GetVertexBuffer() const { return m_VertexBuffer.get(); }
const Buffer *Mesh2::GetIndexBuffer() const { return m_IndexBuffer.get(); }
uint32_t Mesh2::GetVertexCount() const { return m_VertexCount; }
uint32_t Mesh2::GetIndexCount() const { return m_IndexCount; }
vk::IndexType Mesh2::GetIndexType() const { return m_IndexType; }
} // namespace Core
} // namespace SimpleEngine
