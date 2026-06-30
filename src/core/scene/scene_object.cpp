#include "core/scene/scene_object.hpp"
#include "core/component/mesh_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
SceneObject::SceneObject(const std::string &name) : Entity(name) {
  addComponent<TransformComponent>();
  addComponent<MeshComponent>();
}
SceneObject::~SceneObject() {
  // TODO: Destructor for scene object
}

TransformComponent *SceneObject::getTransform() {
  return getComponent<TransformComponent>();
}

MeshComponent *SceneObject::getMesh() { return getComponent<MeshComponent>(); }

} // namespace Core
} // namespace SimpleEngine
