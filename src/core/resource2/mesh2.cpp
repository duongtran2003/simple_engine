#include "core/resource2/mesh2.hpp"
#include "core/buffer/index_buffer.hpp"
#include "core/buffer/vertex_buffer.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace SimpleEngine {
namespace Core {
Mesh2::Mesh2(const std::string &identifier,
             std::unique_ptr<VertexBuffer> vertexBuffer,
             std::unique_ptr<IndexBuffer> indexBuffer,
             vk::IndexType indexType) {
  m_Identifier = identifier;
  m_VertexBuffer = std::move(vertexBuffer);
  m_IndexBuffer = std::move(indexBuffer);
}

Mesh2::~Mesh2() {}

const std::string &Mesh2::GetIdentifier() const { return m_Identifier; }
const VertexBuffer *Mesh2::GetVertexBuffer() const {
  return m_VertexBuffer.get();
}
const IndexBuffer *Mesh2::GetIndexBuffer() const { return m_IndexBuffer.get(); }
uint32_t Mesh2::GetVertexCount() const {
  return m_VertexBuffer->GetVertexCount();
}
uint32_t Mesh2::GetIndexCount() const { return m_IndexBuffer->GetIndexCount(); }
vk::IndexType Mesh2::GetIndexType() const {
  return m_IndexBuffer->GetIndexType();
}
} // namespace Core
} // namespace SimpleEngine
