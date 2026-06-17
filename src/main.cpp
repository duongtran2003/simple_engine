#include "core/app.hpp"
#include <cstdlib>

int main() {
  SimpleEngine::Core::App *app = new SimpleEngine::Core::App();
  app->run();

  return EXIT_SUCCESS;
}
