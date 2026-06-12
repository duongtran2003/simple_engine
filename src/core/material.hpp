#pragma once

#include "core/resource/resource_handle.hpp"
#include "core/resource/shader.hpp"
#include "core/resource/texture.hpp"

namespace SimpleEngine {
namespace Core {
class Material {
private:
  ResourceHandle<Shader> vertexShader;
  ResourceHandle<Shader> fragmentShader;
  ResourceHandle<Texture> diffuse;

public:
  Material(ResourceHandle<Shader> &vertexShader,
           ResourceHandle<Shader> &fragmentShader);

  void setDiffuseTexture(ResourceHandle<Texture> &diffuseTexture);

  ResourceHandle<Shader> getVertexShader() const;
  ResourceHandle<Shader> getFragmentShader() const;
  ResourceHandle<Texture> getDiffuseTexture() const;
};
} // namespace Core
} // namespace SimpleEngine
