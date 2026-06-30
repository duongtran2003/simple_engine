#pragma once

#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
class SceneObject : public Entity {
public:
  SceneObject() = delete;
  SceneObject(const std::string &name);
  ~SceneObject() override;

  TransformComponent *getTransform();
  MeshComponent *getMesh();
};
} // namespace Core
} // namespace SimpleEngine
