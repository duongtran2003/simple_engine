#pragma once

#include "core/camera.hpp"
#include "core/entity/entity.hpp"
#include <vector>

namespace SimpleEngine {
namespace Core {
class CullingSystem {
private:
  Camera *camera;
  std::vector<Entity *> visibleEntities;

public:
  CullingSystem(Camera *camera);
  CullingSystem *setCamera(Camera *camera);

  void cullScene(const std::vector<Entity *> &entities);

  const std::vector<Entity *> &getVisibleEntities() const;
};
} // namespace Core
} // namespace SimpleEngine
