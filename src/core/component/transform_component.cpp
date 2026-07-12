#include "core/component/transform_component.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>

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

void TransformComponent::calculateTransformMatrices() const {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position) *
                    glm::mat4_cast(rotation) *
                    glm::scale(glm::mat4(1.0f), scale);

  isTransformDirty = false;

  transformMatrix = model;
  normalTransformMatrix = glm::transpose(glm::inverse(transformMatrix));
}

glm::mat4 TransformComponent::getTransformMatrix() const {
  if (isTransformDirty) {
    calculateTransformMatrices();
  }
  return transformMatrix;
}

glm::mat4 TransformComponent::getNormalTransformMatrix() const {
  if (isTransformDirty) {
    calculateTransformMatrices();
  }
  return normalTransformMatrix;
}
} // namespace Core
} // namespace SimpleEngine
