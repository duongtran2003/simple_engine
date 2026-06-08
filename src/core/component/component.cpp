#include "core/component/component.hpp"
#include "core/entity/entity.hpp"

namespace SimpleEngine {
namespace Core {
Component::~Component() {
  if (state != State::Destroyed) {
    state = State::Destroying;
    onDestroy();
    state = State::Destroyed;
  }
}

void Component::initialize() {
  if (state == State::Unintialized) {
    state = State::Initialzing;
    onInitialize();
    state = State::Active;
  }
};

void Component::destroy() {
  if (state == State::Active) {
    state = State::Destroying;
    onDestroy();
    state = State::Destroyed;
  }
}

void Component::onInitialize() {};
void Component::onDestroy() {};
void Component::update(float deltaTime) {};
void Component::render() {};

void Component::setOwner(Entity *entity) { owner = entity; }
Entity *Component::getOwner() const { return owner; }

bool Component::isActive() const { return state == State::Active; }
} // namespace Core
} // namespace SimpleEngine
