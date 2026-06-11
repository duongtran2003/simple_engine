#include "core/resource/mesh.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
Mesh::Mesh(const std::string &id) : Resource(id) {}

Mesh::~Mesh() { unload(); }

bool Mesh::loadMeshData(const std::string &path,
                        std::vector<Mesh::Vertex> &vertices,
                        std::vector<uint32_t> &indices) {
  // TODO: Implement later

  return false;
};

void Mesh::createVertexBuffer(std::vector<Mesh::Vertex> &vertices) {
  // TODO: Implement once we have the vkDevice running
}

void Mesh::createIndexBuffer(std::vector<uint32_t> &indices) {
  // TODO: Implement once we have the vkDevice running
}

vk::Buffer Mesh::getVertexBuffer() const { return vertexBuffer; }

vk::Buffer Mesh::getIndexBuffer() const { return indexBuffer; }

uint32_t Mesh::getVertexCount() const { return vertexCount; }

uint32_t Mesh::getIndexCount() const { return indexCount; }

bool Mesh::doLoad() {
  std::string filePath = "models/" + getId() + ".gltf";
  std::vector<Vertex> _vertices;
  std::vector<uint32_t> _indices;

  if (loadMeshData(filePath, _vertices, _indices)) {
    return false;
  }

  createVertexBuffer(_vertices);
  createIndexBuffer(_indices);

  vertexCount = static_cast<uint32_t>(_vertices.size());
  indexCount = static_cast<uint32_t>(_indices.size());

  return true;
}

void Mesh::doUnload() {
  if (isLoaded()) {
    vk::Device device = getDevice();

    device.destroyBuffer(indexBuffer);
    device.freeMemory(indexBufferMemory);

    device.destroyBuffer(vertexBuffer);
    device.freeMemory(vertexBufferMemory);
  }
}
} // namespace Core
} // namespace SimpleEngine
