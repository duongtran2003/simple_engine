#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>

#include "core/component/component.hpp"

namespace SimpleEngine {
namespace Core {
class CameraComponent : public Component {
private:
  float near = 0.1f;
  float far = 100.0f;
  float aspectRatio = 16.0f / 9.0f;
  float fov = 45.0f;

  mutable glm::mat4 projectionMatrix = glm::mat4(1.0f);
  mutable bool isProjectionDirty = false;

public:
  void setPerspective(float near, float far, float aspectRatio, float fov);
  glm::mat4 getProjectionMatrix() const;
  glm::mat4 getViewMatrix() const;
  float getFar() const;
  float getNear() const;
  float getFov() const;
  float getAspectRatio() const;
};
} // namespace Core
} // namespace SimpleEngine
