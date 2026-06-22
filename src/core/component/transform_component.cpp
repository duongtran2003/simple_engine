#include "core/component/transform_component.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SimpleEngine {
namespace Core {
TransformComponent *TransformComponent::setPosition(glm::vec3 pos) {
  position = pos;
  isTransformDirty = true;

  return this;
}

TransformComponent *TransformComponent::setRotation(glm::quat quat) {
  rotation = quat;
  isTransformDirty = true;

  return this;
}

TransformComponent *TransformComponent::setScale(glm::vec3 s) {
  scale = s;
  isTransformDirty = true;

  return this;
}

glm::vec3 TransformComponent::getPosition() const { return position; }

glm::quat TransformComponent::getRotation() const { return rotation; }

glm::vec3 TransformComponent::getScale() const { return scale; }

glm::mat4 TransformComponent::getTransformMatrix() const {
  if (!isTransformDirty) {
    return transformMatrix;
  }

  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
  glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
  glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

  isTransformDirty = false;

  transformMatrix = translationMatrix * rotationMatrix * scaleMatrix;
  return transformMatrix;
}
} // namespace Core
} // namespace SimpleEngine
