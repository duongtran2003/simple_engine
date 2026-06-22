#pragma once

#include "core/component/component.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Entity {
private:
  std::string name;
  bool active = false;
  std::vector<std::unique_ptr<Component>> components;
  std::unordered_map<size_t, Component *> componentsMap;

public:
  Entity(const std::string &name);
  virtual ~Entity() = default;

  const std::string &getName() const;
  bool isActive() const;
  void setActive(bool isActive);

  void initialize();
  virtual void update(float deltaTime);
  void render();

  template <typename T, typename... Args> T *addComponent(Args &&...args);
  template <typename T> bool removeComponent();
  template <typename T> T *getComponent() const;
};

template <typename T, typename... Args>
T *Entity::addComponent(Args &&...args) {
  static_assert(std::is_base_of<Component, T>::value,
                "Added component must derived from Component class");

  size_t typeId = Component::getTypeId<T>();
  auto [mapIt, inserted] = componentsMap.try_emplace(typeId, nullptr);
  if (!inserted) {
    return static_cast<T *>(mapIt->second);
  }

  std::unique_ptr<T> component =
      std::make_unique<T>(std::forward<Args>(args)...);

  T *componentPtr = component.get();
  componentPtr->setOwner(this);

  components.push_back(std::move(component));
  mapIt->second = componentPtr;

  return componentPtr;
}

template <typename T> bool Entity::removeComponent() {
  size_t typeId = Component::getTypeId<T>();
  auto mapIt = componentsMap.find(typeId);

  if (mapIt == componentsMap.end()) {
    return false;
  }

  Component *componentPtr = mapIt->second;
  componentsMap.erase(mapIt);

  for (auto listIt = components.begin(); listIt < components.end(); listIt++) {
    if (listIt->get() == componentPtr) {
      components.erase(listIt);
      return true;
    }
  }

  return false;
}

template <typename T> T *Entity::getComponent() const {
  size_t typeId = Component::getTypeId<T>();
  auto it = componentsMap.find(typeId);

  if (it == componentsMap.end()) {
    return nullptr;
  }
  return static_cast<T *>(it->second);
}
} // namespace Core
} // namespace SimpleEngine
