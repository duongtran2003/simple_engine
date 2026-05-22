#include "core/component/transform_component.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SimpleEngine {
namespace Core {
void TransformComponent::setPosition(glm::vec3 pos) {
  position = pos;
  isTransformDirty = true;
}

void TransformComponent::setRotation(glm::quat quat) {
  rotation = quat;
  isTransformDirty = true;
}
} // namespace Core
} // namespace SimpleEngine
