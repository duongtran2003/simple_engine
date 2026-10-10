#include "core/resource2/mesh2.hpp"
#include "core/buffer/buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SimpleEngine {
namespace Core {
Mesh::Mesh(const std::string &identifier, std::unique_ptr<Buffer> vertexBuffer,
           std::unique_ptr<Buffer> indexBuffer, uint32_t vertexCount,
           uint32_t indexCount, vk::IndexType indexType) {
  m_Identifier = identifier;
  m_VertexBuffer = std::move(vertexBuffer);
  m_IndexBuffer = std::move(indexBuffer);
  m_VertexCount = vertexCount;
  m_IndexCount = indexCount;
  m_IndexType = indexType;
}

Mesh::~Mesh() {}

const std::string &Mesh::GetIdentifier() const { return m_Identifier; }
const Buffer *Mesh::GetVertexBuffer() const { return m_VertexBuffer.get(); }
const Buffer *Mesh::GetIndexBuffer() const { return m_IndexBuffer.get(); }
uint32_t Mesh::GetVertexCount() const { return m_VertexCount; }
uint32_t Mesh::GetIndexCount() const { return m_IndexCount; }
vk::IndexType Mesh::GetIndexType() const { return m_IndexType; }
} // namespace Core
} // namespace SimpleEngine
