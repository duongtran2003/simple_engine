#include "core/resource/resource_manager.hpp"
#include "core/resource/resource.hpp"
#include "core/resource/resource_handle.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>

namespace SimpleEngine {
namespace Core {
template <typename T>
ResourceHandle<T> ResourceManager::load(const std::string &resourceId) {
  static_assert(std::is_base_of<Resource, T>::value,
                "T must derive from Resource base class");

  auto &typeResources = resources[std::type_index(typeid(T))];

  auto [resourceIt, inserted] = typeResources.try_emplace(
      resourceId, ResourceData{.resource = nullptr, .refCount = 1});
  if (!inserted) {
    resourceIt->second.refCount += 1;
    return ResourceHandle<T>(resourceId, this);
  }

  auto resource = std::make_shared<T>(resourceId);
  if (!resource->load()) {
    typeResources.erase(resourceIt);
    return ResourceHandle<T>();
  }

  resourceIt->second = ResourceData{.resource = resource, .refCount = 1};
  return ResourceHandle<T>(resourceId, this);
}

template <typename T>
T *ResourceManager::getResource(const std::string &resourceId) {
  auto &typeResources = resources[std::type_index(typeid(T))];
  auto resourceIt = typeResources.find(resourceId);
  if (resourceIt != typeResources.end()) {
    return static_cast<T *>(resourceIt->second.resource.get());
  }

  return nullptr;
}

template <typename T>
bool ResourceManager::hasResource(const std::string &resourceId) {
  auto &typeResources = resources[std::type_index(typeid(T))];
  auto resourceIt = typeResources.find(resourceId);
  return resourceIt != typeResources.end();
}

template <typename T>
void ResourceManager::release(const std::string &resourceId) {
  auto &typeResources = resources[std::type_index(typeid(T))];
  auto resourceIt = typeResources.find(resourceId);

  if (resourceIt != typeResources.end()) {
    resourceIt->second.refCount -= 1;
    if (resourceIt->second.refCount <= 0) {
      resourceIt->second.resource->unload();
      typeResources.erase(resourceIt);
    }
  }
}

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
