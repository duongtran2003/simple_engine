#pragma once

#include "core/event/event.hpp"

namespace SimpleEngine {
namespace Core {
class EventListener {
public:
  virtual ~EventListener() = default;
  virtual void onEvent(const Event &event) = 0;
};
} // namespace Core
} // namespace SimpleEngine
