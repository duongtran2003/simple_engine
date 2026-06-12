#include "core/component/mesh_component.hpp"
#include "core/material.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_handle.hpp"

namespace SimpleEngine {
namespace Core {
MeshComponent *MeshComponent::setMesh(ResourceHandle<Mesh> &mesh) {
  this->mesh = mesh;
  return this;
}

MeshComponent *MeshComponent::setMaterial(Material *material) {
  this->material = material;
  return this;
}

const ResourceHandle<Mesh> &MeshComponent::getMesh() const { return mesh; }
const Material *MeshComponent::getMaterial() const { return material; }

void MeshComponent::render() {
  // TODO: Implement later
}
} // namespace Core
} // namespace SimpleEngine
