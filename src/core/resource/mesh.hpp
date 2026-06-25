#pragma once

#include "core/raw_texture.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
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
  };

  struct MeshTexture {
    vk::Image image;
    vk::DeviceMemory memory;
    vk::ImageView view;
    vk::Sampler sampler;
  };

private:
  vk::Buffer vertexBuffer;
  vk::DeviceMemory vertexBufferMemory;
  vk::DeviceSize vertexBufferOffset;
  uint32_t vertexCount = 0;

  vk::Buffer indexBuffer;
  vk::DeviceMemory indexBufferMemory;
  vk::DeviceSize indexBufferOffset;
  uint32_t indexCount = 0;
  std::string modelPath;

  std::vector<MeshTexture> meshTextures;

  bool loadMeshData(const std::string &path, std::vector<Vertex> &vertices,
                    std::vector<uint32_t> &indices,
                    std::vector<RawTexture> &textures);

  void createVertexBuffer(std::vector<Vertex> &vertices);
  void createIndexBuffer(std::vector<uint32_t> &indices);
  void createMeshTextures(std::vector<RawTexture> &textures);

public:
  Mesh(const std::string &id, const RenderContext &renderContext,
       const std::string &modelPath);
  ~Mesh() override;

  vk::Buffer getVertexBuffer() const;
  vk::Buffer getIndexBuffer() const;

  uint32_t getVertexCount() const;
  uint32_t getIndexCount() const;

protected:
  bool doLoad() override;
  void doUnload() override;
};
} // namespace Core
} // namespace SimpleEngine
