#pragma once

#include "core/resource/resource_manager.hpp"
#include <string>

namespace SimpleEngine {
namespace Core {
template <typename T> class ResourceHandle {
private:
  std::string resourceId;
  ResourceManager *resourceManager;

public:
  ResourceHandle() { resourceManager = nullptr; }
  ResourceHandle(const std::string &id, ResourceManager *manager) {
    resourceId = id;
    resourceManager = manager;
  }
  ~ResourceHandle() {
    if (resourceManager != nullptr && !resourceId.empty()) {
      resourceManager->release<T>(resourceId);
    }
  }

  T *get() const {
    if (resourceManager == nullptr) {
      return nullptr;
    }

    return resourceManager->getResource<T>(resourceId);
  }

  bool isValid() const {
    if (resourceManager == nullptr) {
      return false;
    }

    return resourceManager->hasResource<T>(resourceId);
  }

  const std::string &getId() const { return resourceId; }

  T *operator->() const { return get(); }
  T &operator*() const { return *get(); }
  operator bool() const { return isValid(); }
};
} // namespace Core
} // namespace SimpleEngine
