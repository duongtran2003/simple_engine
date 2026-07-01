#include "core/resource/resource.hpp"
#include "core/render_context.hpp"
#include <iostream>
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

  std::cout << "Resource::load::INFO: " << resourceId << " loaded "
            << (loaded ? "successfully" : "failed") << "\n";
  return loaded;
}

void Resource::unload() {
  doUnload();
  loaded = false;

  std::cout << "Resource::unload::INFO: " << resourceId << " unloaded.\n";
}
} // namespace Core
} // namespace SimpleEngine
