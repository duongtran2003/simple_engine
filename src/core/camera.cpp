#include "core/camera.hpp"
#include "core/component/camera_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/input/key_code.hpp"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>

namespace SimpleEngine {
namespace Core {
Camera::Camera(const Input &input) : Entity("g_camera"), input(input) {
  addComponent<CameraComponent>();
  addComponent<TransformComponent>();

  TransformComponent *transform = getComponent<TransformComponent>();
  transform->setPosition({0.0f, 0.0f, 5.0f});
}

TransformComponent *Camera::getTransform() const {
  return getComponent<TransformComponent>();
}

CameraComponent *Camera::getCamera() const {
  return getComponent<CameraComponent>();
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

  if (input.isKeyHeld(Key::W)) {
    glm::vec3 position = transform->getPosition();
    position += getForward() * speed;
    transform->setPosition(position);
  } else if (input.isKeyHeld(Key::S)) {
    glm::vec3 position = transform->getPosition();
    position -= getForward() * speed;
    transform->setPosition(position);
  } else if (input.isKeyHeld(Key::A)) {
    glm::vec3 position = transform->getPosition();
    position -= getRight() * speed;
    transform->setPosition(position);
  } else if (input.isKeyHeld(Key::D)) {
    glm::vec3 position = transform->getPosition();
    position += getRight() * speed;
    transform->setPosition(position);
  } else if (input.isKeyHeld(Key::Space)) {
    glm::vec3 position = transform->getPosition();
    position += getUp() * speed;
    transform->setPosition(position);
  } else if (input.isKeyHeld(Key::LeftCtrl)) {
    glm::vec3 position = transform->getPosition();
    position -= getUp() * speed;
    transform->setPosition(position);
  }
}

void Camera::update(float delta) {
  handleInput(delta);
  auto transform = getTransform();
  auto camera = getCamera();
  glm::mat4 modelMatrix = transform->getTransformMatrix();
  glm::mat4 viewMatrix = camera->getViewMatrix();
  glm::mat4 projectionMatrix = camera->getProjectionMatrix();
}
} // namespace Core
} // namespace SimpleEngine
