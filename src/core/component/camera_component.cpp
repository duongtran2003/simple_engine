#include "core/component/camera_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>
#include <iostream>

namespace SimpleEngine {
namespace Core {
CameraComponent *CameraComponent::setFar(float far) {
  this->far = far;
  isProjectionDirty = true;

  return this;
}

CameraComponent *CameraComponent::setNear(float near) {
  this->near = near;
  isProjectionDirty = true;

  return this;
}

CameraComponent *CameraComponent::setAspectRatio(float ratio) {
  aspectRatio = ratio;
  isProjectionDirty = true;

  return this;
}

CameraComponent *CameraComponent::setFov(float fov) {
  this->fov = fov;
  isProjectionDirty = true;

  return this;
}

float CameraComponent::getFar() const { return far; }
float CameraComponent::getNear() const { return near; }
float CameraComponent::getFov() const { return fov; }
float CameraComponent::getAspectRatio() const { return aspectRatio; }

glm::mat4 CameraComponent::getViewMatrix() const {
  Entity *owner = getOwner();
  TransformComponent *transform = owner->getComponent<TransformComponent>();
  if (transform == nullptr) {
    return glm::mat4(1.0f);
  }

  glm::vec3 position = transform->getPosition();
  glm::quat rotationQuat = transform->getRotation();

  glm::vec3 up = rotationQuat * glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 forward = rotationQuat * glm::vec3(0.0f, 0.0f, -1.0f);

  glm::mat4 lookAt = glm::lookAt(position, position + forward, up);
  return lookAt;
}

glm::mat4 CameraComponent::getProjectionMatrix() const {
  if (!isProjectionDirty) {
    return projectionMatrix;
  }

  isProjectionDirty = false;

  float tanGamma = glm::tan(glm::radians(fov / 2.0f));
  projectionMatrix = glm::mat4(0.0f);
  projectionMatrix[0][0] = 1.0f / (tanGamma * aspectRatio);
  projectionMatrix[1][1] = -1.0f / tanGamma;
  projectionMatrix[2][2] = (far) / (near - far);
  projectionMatrix[3][2] = (near * far) / (near - far);
  projectionMatrix[2][3] = -1;

  return projectionMatrix;
}
} // namespace Core
} // namespace SimpleEngine
