#pragma once

#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>

namespace SimpleEngine {
namespace Core {
template <typename T> class ResourceHandle;

class ResourceManager {
private:
  struct ResourceData {
    std::shared_ptr<Resource> resource;
    int refCount;
  };
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, ResourceData>>
      resources;

  const RenderContext &renderContext;

public:
  ResourceManager(const RenderContext &renderContext);

  template <typename T, typename... Args>
  ResourceHandle<T> load(const std::string &resourceId, Args &&...args);

  template <typename T> void acquire(const std::string &resourceId);

  template <typename T> void release(const std::string &resourceId);
  void releaseAll();
  template <typename T> T *getResource(const std::string &resourceId);
  template <typename T> bool hasResource(const std::string &resourceId);
};
} // namespace Core
} // namespace SimpleEngine

#include "core/resource/resource_handle.hpp"

namespace SimpleEngine {
namespace Core {
template <typename T, typename... Args>
ResourceHandle<T> ResourceManager::load(const std::string &resourceId,
                                        Args &&...args) {
  static_assert(std::is_base_of<Resource, T>::value,
                "T must derive from Resource base class");

  auto &typeResources = resources[std::type_index(typeid(T))];

  auto [resourceIt, inserted] = typeResources.try_emplace(
      resourceId, ResourceData{.resource = nullptr, .refCount = 1});
  if (!inserted) {
    resourceIt->second.refCount += 1;
    return ResourceHandle<T>(resourceId, this);
  }

  auto resource = std::make_shared<T>(resourceId, renderContext,
                                      std::forward<Args>(args)...);
  if (!resource->load()) {
    typeResources.erase(resourceIt);
    return ResourceHandle<T>();
  }

  resourceIt->second = ResourceData{.resource = resource, .refCount = 1};
  return ResourceHandle<T>(resourceId, this);
}

template <typename T>
void ResourceManager::acquire(const std::string &resourceId) {
  auto &typeResources = resources[std::type_index(typeid(T))];
  auto it = typeResources.find(resourceId);

  if (it != typeResources.end()) {
    std::cout << "ResourceManager::acquire::INFO: Acquiring " + resourceId +
                     "\n";
    it->second.refCount += 1;
  }
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
      std::cout << "ResourceManager::release::INFO: Unloading " + resourceId +
                       "\n";
      resourceIt->second.resource->unload();
      typeResources.erase(resourceIt);
    }
  }
}
} // namespace Core
} // namespace SimpleEngine
