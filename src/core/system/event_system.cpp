#include "core/system/event_system.hpp"
#include "core/event/event.hpp"
#include "core/event/event_listener.hpp"
namespace SimpleEngine {
namespace Core {
void EventSystem::addListener(EventListener *listener) {
  listeners.push_back(listener);
}

void EventSystem::removeListener(EventListener *listener) {
  std::erase_if(listeners, [listener](EventListener *_listener) {
    return _listener == listener;
  });
}

void EventSystem::dispatchEvent(const Event &event) {
  for (EventListener *listener : listeners) {
    listener->onEvent(event);
  }
}
} // namespace Core
} // namespace SimpleEngine
