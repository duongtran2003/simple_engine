#include "core/resource/resource_manager.hpp"
#include "core/render_context.hpp"

namespace SimpleEngine {
namespace Core {

ResourceManager::ResourceManager(const RenderContext &renderContext)
    : renderContext(renderContext) {};

void ResourceManager::releaseAll() {
  for (auto &[type, typeResources] : resources) {
    for (auto it = typeResources.begin(); it != typeResources.end(); it++) {
      it->second.refCount = 0;
      it->second.resource->unload();
    }

    typeResources.clear();
  }
}
} // namespace Core
} // namespace SimpleEngine
