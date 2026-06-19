#include "core/resource/resource.hpp"
#include "core/render_context.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
Resource::Resource(const std::string &id, const RenderContext &renderContext)
    : renderContext(renderContext) {
  resourceId = id;
}
const std::string &Resource::getId() const { return resourceId; }
const RenderContext &Resource::getRenderContext() const {
  return renderContext;
}
bool Resource::isLoaded() const { return loaded; }

bool Resource::load() {
  loaded = doLoad();
  return loaded;
}

void Resource::unload() {
  doUnload();
  loaded = false;
}
} // namespace Core
} // namespace SimpleEngine
