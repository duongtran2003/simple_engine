#pragma once

#include "core/component/component.hpp"
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace SimpleEngine {
namespace Core {
class Entity {
private:
  std::string name;
  bool active = false;
  std::vector<std::unique_ptr<Component>> components;

public:
  Entity(const std::string &name);

  const std::string &getName() const;
  bool isActive() const;
  void setActive(bool isActive);

  void initialize();
  void update(float deltaTime);
  void render();

  template <typename T, typename... Args> T *addComponent(Args &&...args);
  template <typename T> bool removeComponent();
  template <typename T> T *getComponent() const;
};

template <typename T, typename... Args>
T *Entity::addComponent(Args &&...args) {
  static_assert(std::is_base_of<Component, T>::value,
                "Added component must derived from Component class");

  std::unique_ptr<T> component =
      std::make_unique<T>(std::forward<Args>(args)...);

  T *componentPt = component.get();
  componentPt->setOwner(this);
  components.push_back(std::move(component));

  return componentPt;
}

template <typename T> bool Entity::removeComponent() {
  for (auto it = components.begin(); it != components.end(); it++) {
    if (dynamic_cast<T *>(it->get())) {
      components.erase(it);
      return true;
    }
  }

  return false;
}

template <typename T> T *Entity::getComponent() const {
  for (auto &component : components) {
    if (T *result = dynamic_cast<T *>(component.get())) {
      return result;
    }
  }

  return nullptr;
}
} // namespace Core
} // namespace SimpleEngine
