#pragma once

#include <string>
namespace SimpleEngine {
namespace Helper {
class ModelLoader {
public:
  ModelLoader() = delete;

  static void loadglTF(const std::string& path);
};
} // namespace Helper
} // namespace SimpleEngine
