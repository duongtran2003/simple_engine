#pragma once

#include "core/component/component.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SimpleEngine {
namespace Core {
class TransformComponent : public Component {
private:
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale = glm::vec3(0.0f);

  glm::mat4 mutable transformMatrix = glm::mat4(1.0f);
  bool mutable isTransformDirty = false;

public:
  TransformComponent *setPosition(glm::vec3 pos);
  TransformComponent *setRotation(glm::quat rot);
  TransformComponent *setScale(glm::vec3 s);

  glm::mat4 getTransformMatrix() const;
  glm::vec3 getPosition() const;
  glm::quat getRotation() const;
  glm::vec3 getScale() const;
};
} // namespace Core
} // namespace SimpleEngine
