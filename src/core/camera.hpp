#pragma once

#include "core/component/camera_component.hpp"
#include "core/component/transform_component.hpp"
#include "core/entity/entity.hpp"
#include "core/input/input.hpp"
#include <glm/ext/vector_float3.hpp>

namespace SimpleEngine {
namespace Core {

class Camera : public Entity {
public:
  Camera() = delete;
  virtual ~Camera() = default;

  Camera(const Input &input);
  void update(float delta) override;

  TransformComponent *getTransform() const;
  CameraComponent *getCamera() const;

  glm::vec3 getUp() const;
  glm::vec3 getRight() const;
  glm::vec3 getForward() const;
private:
  const Input &input;
  float velocity = 10.0f;

  void handleInput(float delta);
};
} // namespace Core
} // namespace SimpleEngine
