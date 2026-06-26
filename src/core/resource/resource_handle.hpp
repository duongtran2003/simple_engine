#pragma once

#include "core/resource/resource_manager.hpp"
#include <iostream>
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

  // Copy constructor
  ResourceHandle(const ResourceHandle &other) {
    this->resourceId = other.resourceId;
    this->resourceManager = other.resourceManager;

    if (this->resourceManager && !this->resourceId.empty()) {
      this->resourceManager->acquire<T>(this->resourceId);
    }
  }

  ResourceHandle &operator=(const ResourceHandle &other) {
    if (this == &other) {
      return *this;
    }

    if (this->resourceManager && !this->resourceId.empty()) {
      this->resourceManager->release<T>(this->resourceId);
    }

    this->resourceId = other.resourceId;
    this->resourceManager = other.resourceManager;

    if (this->resourceManager && !this->resourceId.empty()) {
      this->resourceManager->acquire<T>(this->resourceId);
    }

    return *this;
  }

  ~ResourceHandle() {
    if (resourceManager != nullptr && !resourceId.empty()) {
      std::cout << "ResourceHandle::~ResourceHandle::INFO: Releasing "
                << resourceId << "\n";
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
