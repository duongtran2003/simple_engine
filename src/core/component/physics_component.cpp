#include "core/component/physics_component.hpp"
#include "core/event/collision_event.hpp"
#include "core/event/event.hpp"
#include "core/system/event_system.hpp"
#include <iostream>

namespace SimpleEngine {
namespace Core {
void PhysicsComponent::initialize() { getEventSystem().addListener(this); }
PhysicsComponent::~PhysicsComponent() { getEventSystem().removeListener(this); }

void PhysicsComponent::onEvent(const Event &event) {
  const CollisionEvent *collisionEvent =
      dynamic_cast<const CollisionEvent *>(&event);

  if (collisionEvent != nullptr) {
    std::cout << "Handle collision event";
  }
}

EventSystem &PhysicsComponent::getEventSystem() {
  // Placeholder, might change later
  static EventSystem eventSystem;
  return eventSystem;
}
} // namespace Core
} // namespace SimpleEngine
