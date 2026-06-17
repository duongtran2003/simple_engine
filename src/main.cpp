#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define GFLW_INCLUDE_VULKAN
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "core/app.hpp"
#include <cstdlib>

int main() {
  SimpleEngine::Core::App *app = new SimpleEngine::Core::App();
  app->run();

  return EXIT_SUCCESS;
}
