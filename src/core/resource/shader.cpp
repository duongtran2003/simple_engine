#include "core/resource/shader.hpp"
#include "core/resource/resource.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
Shader::Shader(const std::string &id, vk::ShaderStageFlagBits shaderStage)
    : Resource(id) {
  stage = shaderStage;
}
Shader::~Shader() { unload(); }

bool Shader::doLoad() {
  std::string extension;
  switch (stage) {
  case vk::ShaderStageFlagBits::eVertex:
    extension = ".vert";
    break;
  case vk::ShaderStageFlagBits::eFragment:
    extension = ".frag";
    break;
  case vk::ShaderStageFlagBits::eCompute:
    extension = ".comp";
    break;
  default:
    return false;
  }

  std::string filePath = "shaders/" + getId() + extension + ".spv";

  std::vector<char> shaderCode;
  if (!readShaderFile(filePath, shaderCode)) {
    return false;
  }

  createShaderModule(shaderCode);

  return true;
}

void Shader::doUnload() {
  if (isLoaded()) {
    vk::Device device = getDevice();
    device.destroyShaderModule(shaderModule);
  }
}

vk::ShaderModule Shader::getShaderModule() const { return shaderModule; }
vk::ShaderStageFlagBits Shader::getShaderStage() const { return stage; }

bool Shader::readShaderFile(const std::string &path,
                            std::vector<char> &buffer) {
  std::ifstream file(path, std::ios::ate | std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open shader file");
  }

  buffer.reserve(file.tellg());
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();

  return true;
}

void Shader::createShaderModule(const std::vector<char> &shaderCode) {
  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = shaderCode.size(),
      .pCode = reinterpret_cast<const uint32_t *>(shaderCode.data())};

  vk::Device device = getDevice();
  shaderModule = device.createShaderModule(createInfo);
}

} // namespace Core
} // namespace SimpleEngine
