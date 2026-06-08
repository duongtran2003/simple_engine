#pragma once

#include "core/event/event.hpp"
#include "core/event/event_listener.hpp"
#include <vector>

namespace SimpleEngine {
namespace Core {
class EventSystem {
private:
  std::vector<EventListener *> listeners;

public:
  void addListener(EventListener *listener);
  void removeListener(EventListener *listener);
  void dispatchEvent(const Event &event);
};
} // namespace Core
} // namespace SimpleEngine
