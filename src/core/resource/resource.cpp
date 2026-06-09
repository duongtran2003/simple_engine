#include "core/resource/resource.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
Resource::Resource(const std::string &id) { resourceId = id; }
const std::string &Resource::getId() const { return resourceId; }
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
