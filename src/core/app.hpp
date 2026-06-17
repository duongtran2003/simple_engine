#pragma once

#include "core/render_context.hpp"

namespace SimpleEngine {
namespace Core {
class App {
private:
  RenderContext renderContext;

public:
  App();
  void run();
};
} // namespace Core
} // namespace SimpleEngine
