#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#define GLM_ENABLE_EXPERIMENTAL
#define VULKAN_HPP_STORAGE_SHARED
#define VULKAN_HPP_STORAGE_SHARED_EXPORT

#include <stb_image.h>

#include <cstdlib>
#include <exception>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_hpp_macros.hpp>

#include "core/engine.hpp"
#include <iostream>
#include <stdexcept>

#include <filesystem>
#include <iostream>
#include <vulkan/vulkan.hpp>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <limits.h>
#include <unistd.h>
#endif

void setWorkingDirectory() {
  std::filesystem::path exeDir;
#if defined(_WIN32)
  wchar_t path[MAX_PATH];
  GetModuleFileNameW(NULL, path, MAX_PATH);
  exeDir = std::filesystem::path(path).parent_path();
#elif defined(__linux__)
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count > 0) {
    exeDir = std::filesystem::path(std::string(result, count)).parent_path();
  }
#endif

  if (!exeDir.empty()) {
    std::filesystem::current_path(exeDir);
  }
}

int main() {
  setWorkingDirectory();
  try {
    vk::detail::DispatchLoaderDynamic &dld =
        vk::detail::defaultDispatchLoaderDynamic;

    vk::detail::DynamicLoader dl;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

    if (!vkGetInstanceProcAddr) {
      throw std::runtime_error("Failed to load root vkGetInstanceProcAddr!");
    }

    dld.init(vkGetInstanceProcAddr);

    SimpleEngine::Core::Engine engine;
    engine.run();

  } catch (const std::exception &e) {
    std::cerr << "CRITICAL APPLICATION EXCEPTION: " << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
