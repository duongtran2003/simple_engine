#include "core/resource2/resource_manager2.hpp"
#include "core/buffer/index_buffer.hpp"
#include "core/buffer/vertex_buffer.hpp"
#include "core/image/image.hpp"
#include "core/image/image_handle.hpp"
#include "core/image/image_pool.hpp"
#include "core/image/sampler_registry.hpp"
#include "core/render_context.hpp"
#include "core/resource2/mesh2.hpp"
#include "core/resource2/mesh_handle.hpp"
#include "core/resource2/texture2.hpp"
#include "core/resource2/texture_handle.hpp"
#include "enums/image_enums.hpp"
#include "helpers/asset_loader.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SimpleEngine {
namespace Core {
ResourceManager2::ResourceManager2(ImagePool *imagePool,
                                   SamplerRegistry *samplerRegistry,
                                   const RenderContext *renderContext) {
  m_pImagePool = imagePool;
  m_pSamplerRegistry = samplerRegistry;
  m_pContext = renderContext;

  InitTextureSlots();
  InitMeshSlots();
}

void ResourceManager2::InitTextureSlots() {
  m_TextureSlots.resize(MAX_TEXTURES_POOL_SIZE);
  m_TextureFreeList.reserve(MAX_TEXTURES_POOL_SIZE);
  for (int64_t i = MAX_TEXTURES_POOL_SIZE - 1; i >= 1; --i) {
    m_TextureFreeList.push_back(static_cast<uint32_t>(i));
  }
}

void ResourceManager2::InitMeshSlots() {
  m_MeshSlots.resize(MAX_MESHES_POOL_SIZE);
  m_MeshFreeList.reserve(MAX_MESHES_POOL_SIZE);
  for (int64_t i = MAX_MESHES_POOL_SIZE - 1; i >= 1; --i) {
    m_MeshFreeList.push_back(static_cast<uint32_t>(i));
  }
}

ResourceManager2::~ResourceManager2() {
  for (uint32_t i = 1; i < m_TextureSlots.size(); ++i) {
    if (m_TextureSlots[i].RefCount > 0) {
      FreeTextureSlot(i);
    }
  }
  for (uint32_t i = 1; i < m_MeshSlots.size(); ++i) {
    if (m_MeshSlots[i].RefCount > 0) {
      FreeMeshSlot(i);
    }
  }
}

const Texture2 *ResourceManager2::Get(TextureHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_TextureSlots.size()) {
    return nullptr;
  }

  const TextureSlot &slot = m_TextureSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return nullptr;
  }

  return &slot.Texture;
}

const Mesh2 *ResourceManager2::Get(MeshHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_MeshSlots.size()) {
    return nullptr;
  }

  const MeshSlot &slot = m_MeshSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return nullptr;
  }

  return &slot.Mesh;
}

TextureHandle
ResourceManager2::AllocateTexture(const std::string &path,
                                  const TextureAllocateInfo &allocateInfo) {
  auto it = m_TextureCache.find(path);
  if (it != m_TextureCache.end()) {
    return AddRef(it->second);
  }

  if (m_TextureFreeList.size() == 0) {
    return TextureHandle{0, 0};
  }

  uint32_t freeSlotIdx = m_TextureFreeList.back();
  m_TextureFreeList.pop_back();

  TextureSlot &freeSlot = m_TextureSlots[freeSlotIdx];

  std::vector<unsigned char> pixels;
  int width, height, channels;
  Helper::AssetLoader::LoadImage(path, pixels, width, height, channels);
  vk::Format format;
  format = allocateInfo.ColorSpace == Enums::Image::ColorSpace::eLinear
               ? vk::Format::eR8G8B8A8Unorm
               : vk::Format::eR8G8B8A8Srgb;

  Image::CreateInfo imageCreateInfo{
      .Type = vk::ImageType::e2D,
      .Format = format,
      .Extent = vk::Extent3D{static_cast<uint32_t>(width),
                             static_cast<uint32_t>(height), 1},
      .MipLevels = allocateInfo.MipLevels,
      .ArrayLayers = allocateInfo.ArrayLayers,
      .Usage = vk::ImageUsageFlagBits::eTransferSrc |
               vk::ImageUsageFlagBits::eTransferDst |
               vk::ImageUsageFlagBits::eSampled,
      .AspectMask = allocateInfo.AspectMask,
      .Width = static_cast<uint32_t>(width),
      .Height = static_cast<uint32_t>(height),
      .Channels = static_cast<uint32_t>(channels),
      .ColorSpace = allocateInfo.ColorSpace};
  ImageHandle imageHandle = m_pImagePool->Allocate(imageCreateInfo);
  Image *image = m_pImagePool->Get(imageHandle);
  if (image != nullptr) {
    image->UploadData(pixels.data(), pixels.size());
  }

  Texture2 texture = Texture2(path, imageHandle);
  freeSlot.Texture = texture;
  freeSlot.RefCount = 1;

  TextureHandle handle{.Id = freeSlotIdx, .Generation = freeSlot.Generation};
  m_TextureCache[path] = handle;
  return handle;
}

void ResourceManager2::ReleaseTexture(TextureHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_TextureSlots.size()) {
    return;
  }

  TextureSlot &slot = m_TextureSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return;
  }

  slot.RefCount--;
  if (slot.RefCount == 0) {
    FreeTextureSlot(handle.Id);
  }
}

void ResourceManager2::FreeTexture(TextureHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_TextureSlots.size()) {
    return;
  }

  TextureSlot &slot = m_TextureSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return;
  }

  slot.RefCount = 0;
  FreeTextureSlot(handle.Id);
}

void ResourceManager2::FreeTextureSlot(uint32_t slotIndex) {
  TextureSlot &slot = m_TextureSlots[slotIndex];
  std::string path = slot.Texture.GetPath();
  m_TextureCache.erase(path);

  ImageHandle imageHandle = slot.Texture.GetImageHandle();
  if (imageHandle.IsValid()) {
    m_pImagePool->Free(imageHandle);
  }

  slot.Generation++;
  slot.RefCount = 0;
  slot.Texture = Texture2();

  m_TextureFreeList.push_back(slotIndex);
}

MeshHandle
ResourceManager2::AllocateMesh(const std::string &identifier,
                               const MeshAllocateInfo &allocateInfo) {
  auto it = m_MeshCache.find(identifier);
  if (it != m_MeshCache.end()) {
    return AddRef(it->second);
  }

  if (m_MeshFreeList.size() == 0) {
    return MeshHandle{0, 0};
  }

  uint32_t freeSlotIdx = m_MeshFreeList.back();
  m_MeshFreeList.pop_back();

  MeshSlot &freeSlot = m_MeshSlots[freeSlotIdx];

  auto vertexBuffer =
      std::make_unique<VertexBuffer>(allocateInfo.Vertices, m_pContext);
  auto indexBuffer =
      std::make_unique<IndexBuffer>(allocateInfo.Indices, m_pContext);

  Mesh2 mesh = Mesh2(identifier, std::move(vertexBuffer),
                     std::move(indexBuffer), allocateInfo.IndexType);

  freeSlot.Mesh = std::move(mesh);
  freeSlot.RefCount = 1;

  MeshHandle handle{.Id = freeSlotIdx, .Generation = freeSlot.Generation};
  m_MeshCache[identifier] = handle;
  return handle;
}

void ResourceManager2::ReleaseMesh(MeshHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_MeshSlots.size()) {
    return;
  }

  MeshSlot &slot = m_MeshSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return;
  }

  slot.RefCount--;
  if (slot.RefCount == 0) {
    FreeMeshSlot(handle.Id);
  }
}

void ResourceManager2::FreeMesh(MeshHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_MeshSlots.size()) {
    return;
  }

  MeshSlot &slot = m_MeshSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return;
  }

  slot.RefCount = 0;
  FreeMeshSlot(handle.Id);
}

void ResourceManager2::FreeMeshSlot(uint32_t slotIndex) {
  MeshSlot &slot = m_MeshSlots[slotIndex];
  std::string path = slot.Mesh.GetIdentifier();
  m_MeshCache.erase(path);

  slot.Generation++;
  slot.RefCount = 0;
  slot.Mesh = Mesh2();

  m_MeshFreeList.push_back(slotIndex);
}

TextureHandle ResourceManager2::AddRef(TextureHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_TextureSlots.size()) {
    return {0, 0};
  }

  auto &slot = m_TextureSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return {0, 0};
  }

  slot.RefCount++;
  return handle;
}

MeshHandle ResourceManager2::AddRef(MeshHandle handle) {
  if (handle.Id == 0 || handle.Id >= m_MeshSlots.size()) {
    return {0, 0};
  }

  auto &slot = m_MeshSlots[handle.Id];
  if (slot.Generation != handle.Generation || slot.RefCount == 0) {
    return {0, 0};
  }

  slot.RefCount++;
  return handle;
}
} // namespace Core
} // namespace SimpleEngine
