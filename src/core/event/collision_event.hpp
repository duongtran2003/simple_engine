#pragma once

#include "core/entity/entity.hpp"
#include "core/event/event.hpp"

namespace SimpleEngine {
namespace Core {
class CollisionEvent : public Event {
private:
  Entity *entity1;
  Entity *entity2;

public:
  CollisionEvent(Entity *e1, Entity *e2);
  Entity *getEntity1() const;
  Entity *getEntity2() const;
};
} // namespace Core
} // namespace SimpleEngine
