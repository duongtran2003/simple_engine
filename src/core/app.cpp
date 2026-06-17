#include "core/app.hpp"
#include "core/render_context.hpp"
#include <iostream>
namespace SimpleEngine {
namespace Core {
App::App() { renderContext = RenderContext(); }

void App::run() { std::cout << "App run\n"; }
} // namespace Core
} // namespace SimpleEngine
