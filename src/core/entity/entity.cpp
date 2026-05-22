#include "core/entity/entity.hpp"
#include "core/component/component.hpp"
#include <cassert>
#include <memory>
#include <string>

namespace SimpleEngine {
namespace Core {
Entity::Entity(const std::string &name) { this->name = name; };

const std::string &Entity::getName() const { return name; }
bool Entity::isActive() const { return active; }
void Entity::setActive(bool isActive) { active = isActive; }

void Entity::initialize() {
  for (const auto &component : components) {
    component->initialize();
  }
}

void Entity::update(float deltaTime) {
  for (const auto &component : components) {
    component->update(deltaTime);
  }
}

void Entity::render() {
  for (const auto &component : components) {
    component->render();
  }
}
} // namespace Core
} // namespace SimpleEngine
