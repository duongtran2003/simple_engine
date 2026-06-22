#include "core/camera.hpp"
#include "core/component/camera_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include "core/input/key_code.hpp"
#include <iostream>

namespace SimpleEngine {
namespace Core {
Camera::Camera(const Input &input) : Entity("g_camera"), input(input) {
  addComponent<CameraComponent>();
  addComponent<TransformComponent>();
}

void Camera::handleInput(float delta) {
  if (input.isKeyHeld(Key::W)) {
    std::cout << "Move forward with delta " << delta << "\n";
  }
}

void Camera::update(float delta) { handleInput(delta); }
} // namespace Core
} // namespace SimpleEngine
