#include "core/event/collision_event.hpp"
#include "core/entity/entity.hpp"
namespace SimpleEngine {
namespace Core {
CollisionEvent::CollisionEvent(Entity *e1, Entity *e2) {
  entity1 = e1;
  entity2 = e2;
}

Entity *CollisionEvent::getEntity1() const { return entity1; }
Entity *CollisionEvent::getEntity2() const { return entity2; }
} // namespace Core
} // namespace SimpleEngine
