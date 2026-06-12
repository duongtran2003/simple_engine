#include "core/material.hpp"
#include "core/resource/resource_handle.hpp"
#include "core/resource/shader.hpp"
#include "core/resource/texture.hpp"

namespace SimpleEngine {
namespace Core {
Material::Material(ResourceHandle<Shader> &vertexShader,
                   ResourceHandle<Shader> &fragmentShader) {
  this->vertexShader = vertexShader;
  this->fragmentShader = fragmentShader;
}

void Material::setDiffuseTexture(ResourceHandle<Texture> &diffuseTexture) {
  diffuse = diffuseTexture;
}

ResourceHandle<Shader> Material::getVertexShader() const {
  return vertexShader;
}

ResourceHandle<Shader> Material::getFragmentShader() const {
  return fragmentShader;
}

ResourceHandle<Texture> Material::getDiffuseTexture() const { return diffuse; }
} // namespace Core
} // namespace SimpleEngine
