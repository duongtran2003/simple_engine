#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/types.h>
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

std::string GetCurrentDateString() {
  auto now = std::chrono::system_clock::now();
  std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  return ss.str();
}

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

  std::ofstream logFile;
  std::streambuf *origCoutBuf = std::cout.rdbuf();
  std::streambuf *origCerrBuf = std::cerr.rdbuf();

#if !defined(_DEBUG) && defined(NDEBUG)
  try {
    std::filesystem::create_directories("logs");
    std::string logFileName = "logs/engine_" + GetCurrentDateString() + ".log";

    logFile.open(logFileName, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
      std::cout.rdbuf(logFile.rdbuf());
      std::cerr.rdbuf(logFile.rdbuf());
    }
  } catch (const std::exception &e) {
    std::cerr << "Failed to setup log file: " << e.what() << std::endl;
  }
#endif
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

#if !defined(_DEBUG) && defined(NDEBUG)
  if (logFile.is_open()) {
    std::cout.flush();
    std::cerr.flush();
    std::cout.rdbuf(origCoutBuf);
    std::cerr.rdbuf(origCerrBuf);
    logFile.close();
  }
#endif

  return EXIT_SUCCESS;
}
