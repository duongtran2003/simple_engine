#include "core/scene/scene.hpp"
#include "core/camera.hpp"
#include "core/render_context.hpp"
#include "core/scene/scene_object.hpp"
#include "core/system/culling_system.hpp"
#include <iostream>
#include <memory>
#include <utility>

namespace SimpleEngine {
namespace Core {
Scene::Scene(const RenderContext &context, const Camera &camera,
             CullingSystem &cullingSystem)
    : context(context), camera(camera), cullingSystem(cullingSystem) {
  std::cout << "Scene::INFO: Scene initialized.\n";
}

Scene::~Scene() {
  // TODO: Scene object desctructor
}

void Scene::addSceneObject(std::unique_ptr<SceneObject> object) {
  sceneObjects.push_back(std::move(object));
}
} // namespace Core
} // namespace SimpleEngine
