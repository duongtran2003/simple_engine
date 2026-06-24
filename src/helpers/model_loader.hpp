#pragma once

#include "core/resource/mesh.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Helper {
class ModelLoader {
public:
  ModelLoader() = delete;

  static void loadglTF(const std::string &path,
                       std::vector<Core::Mesh::Vertex> &vertices,
                       std::vector<uint32_t> &indices);

  static void loadGlbTextures(const std::string &path);
};
} // namespace Helper
} // namespace SimpleEngine
