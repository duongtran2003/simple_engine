#pragma once

#include "core/component/component.hpp"
#include "core/material.hpp"
#include "core/resource/mesh.hpp"
#include "core/resource/resource_handle.hpp"

namespace SimpleEngine {
namespace Core {
class MeshComponent : public Component {
private:
  ResourceHandle<Mesh> mesh;
  Material material;

public:
  MeshComponent *setMesh(ResourceHandle<Mesh> &mesh);
  MeshComponent *setMaterial(Material &material);

  const ResourceHandle<Mesh> &getMesh() const;
  const Material &getMaterial() const;

protected:
  void render() override;
};
} // namespace Core
} // namespace SimpleEngine
