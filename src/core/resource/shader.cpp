#include "core/resource/shader.hpp"
#include "core/render_context.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
Shader::Shader(const std::string &id, const RenderContext &renderContext,
               vk::ShaderStageFlagBits shaderStage)
    : Resource(id, renderContext) {
  stage = shaderStage;
}
Shader::~Shader() { unload(); }

bool Shader::doLoad() {
  std::string filePath = "shaders/" + getId() + ".spv";

  std::vector<char> shaderCode;
  if (!readShaderFile(filePath, shaderCode)) {
    return false;
  }

  createShaderModule(shaderCode);

  return true;
}

void Shader::doUnload() {
  if (isLoaded()) {
    vk::Device device = getRenderContext().device;
    device.destroyShaderModule(shaderModule);
  }
}

vk::ShaderModule Shader::getShaderModule() const { return shaderModule; }
vk::ShaderStageFlagBits Shader::getShaderStage() const { return stage; }

bool Shader::readShaderFile(const std::string &path,
                            std::vector<char> &buffer) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file with path " + path);
  }

  buffer.resize(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();

  return true;
}

void Shader::createShaderModule(const std::vector<char> &shaderCode) {
  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = shaderCode.size(),
      .pCode = reinterpret_cast<const uint32_t *>(shaderCode.data())};

  const RenderContext &renderContext = getRenderContext();
  shaderModule = renderContext.device.createShaderModule(createInfo);
}

} // namespace Core
} // namespace SimpleEngine
