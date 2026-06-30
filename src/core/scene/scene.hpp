#pragma once

#include "core/camera.hpp"
#include "core/render_context.hpp"
#include "core/scene/scene_object.hpp"
#include "core/system/culling_system.hpp"
#include <memory>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Scene {
private:
  const RenderContext &context;
  const Camera &camera;
  CullingSystem &cullingSystem;

  std::vector<std::unique_ptr<SceneObject>> sceneObjects;

public:
  Scene() = delete;
  Scene(const RenderContext &context, const Camera &camera,
        CullingSystem &cullingSystem);
  ~Scene();

  void addSceneObject(std::unique_ptr<SceneObject> object);
};
} // namespace Core
} // namespace SimpleEngine
