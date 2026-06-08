#pragma once

#include "core/component/component.hpp"
#include "core/event/event.hpp"
#include "core/event/event_listener.hpp"
#include "core/system/event_system.hpp"

namespace SimpleEngine {
namespace Core {
class PhysicsComponent : public Component, public EventListener {
public:
  void initialize() override;
  ~PhysicsComponent() override;
  void onEvent(const Event &event) override;

private:
  EventSystem &getEventSystem();
};
} // namespace Core
} // namespace SimpleEngine
