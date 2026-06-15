#include "core/system/culling_system.hpp"
#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include <vector>

namespace SimpleEngine {
namespace Core {
CullingSystem::CullingSystem(Camera *camera) { this->camera = camera; }

CullingSystem *CullingSystem::setCamera(Camera *camera) {
  this->camera = camera;
  return this;
}

void CullingSystem::cullScene(const std::vector<Entity *> &entities) {
  // TODO: Implement frustum culling
  visibleEntities = entities;
}

const std::vector<Entity *> &CullingSystem::getVisibleEntities() const {
  return visibleEntities;
}
} // namespace Core
} // namespace SimpleEngine
