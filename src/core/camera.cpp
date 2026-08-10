#include "core/camera.hpp"
#include "core/component/camera_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "enums/input.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace SimpleEngine {
namespace Core {
Camera::Camera(const Input &input) : Entity("g_camera"), input(input) {
  addComponent<CameraComponent>();
  addComponent<TransformComponent>();

  TransformComponent *transform = getComponent<TransformComponent>();
  transform->setPosition({0.0f, 0.0f, 4.0f});
}

TransformComponent *Camera::getTransform() const {
  return getComponent<TransformComponent>();
}

CameraComponent *Camera::getCamera() const {
  return getComponent<CameraComponent>();
}

Camera *Camera::setVFov(float fov) {
  CameraComponent *camera = getComponent<CameraComponent>();
  camera->setFov(fov);
  return this;
}

Camera *Camera::setAspectRatio(float aspect) {
  CameraComponent *camera = getComponent<CameraComponent>();
  camera->setAspectRatio(aspect);
  return this;
}

glm::vec3 Camera::getUp() const {
  glm::quat rotationQuat = getTransform()->getRotation();
  return rotationQuat * glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera::getRight() const {
  glm::quat rotationQuat = getTransform()->getRotation();
  return rotationQuat * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Camera::getForward() const {
  glm::quat rotationQuat = getTransform()->getRotation();
  return rotationQuat * glm::vec3(0.0f, 0.0f, -1.0f);
}

void Camera::handleInput(float delta) {
  float speed = velocity * delta;
  auto transform = getTransform();

  glm::vec3 moveDir = glm::vec3(0.0f);

  using eKey = Enums::Input::Key;
  if (input.isKeyHeld(eKey::W)) {
    moveDir += getForward();
  }
  if (input.isKeyHeld(eKey::S)) {
    moveDir -= getForward();
  }
  if (input.isKeyHeld(eKey::A)) {
    moveDir -= getRight();
  }
  if (input.isKeyHeld(eKey::D)) {
    moveDir += getRight();
  }
  if (input.isKeyHeld(eKey::Space)) {
    moveDir += getUp();
  }
  if (input.isKeyHeld(eKey::LeftCtrl)) {
    moveDir -= getUp();
  }

  if (glm::length(moveDir) > 0.00001f) {
    moveDir = glm::normalize(moveDir);

    glm::vec3 position = transform->getPosition();
    transform->setPosition(position + moveDir * speed);
  }

  if (input.isMouseLocked()) {
    glm::vec2 mouseDelta = input.getMouseDelta();
    float yaw = glm::radians(-mouseDelta.x * mouseSensitivity.x);
    float pitch = glm::radians(-mouseDelta.y * mouseSensitivity.y);

    glm::quat yawQuat = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchQuat = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));

    glm::quat rotation = transform->getRotation();
    glm::quat newRotation = yawQuat * rotation * pitchQuat;
    newRotation = glm::normalize(newRotation);

    transform->setRotation(newRotation);
  }
}

void Camera::update(float delta) { handleInput(delta); }
} // namespace Core
} // namespace SimpleEngine
