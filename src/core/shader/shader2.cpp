#include "core/shader/shader2.hpp"
#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <vector>

namespace SimpleEngine {
namespace Core {
Shader2::Shader2(const std::string &path, vk::ShaderStageFlags shaderStage,
                 const RenderContext *pRenderContext) {
  m_Path = path;
  m_ShaderStage = shaderStage;
  m_pContext = pRenderContext;
}

Shader2::~Shader2() {
  if (IsReady()) {
    m_pContext->device.destroyShaderModule(m_ShaderModule);
  }
}

const std::string &Shader2::GetShaderPath() const { return m_Path; }
vk::ShaderStageFlags Shader2::GetShaderStage() const { return m_ShaderStage; }
vk::ShaderModule Shader2::GetShaderModule() const { return m_ShaderModule; }
bool Shader2::IsReady() const { return m_ShaderModule != nullptr; }

void Shader2::CreateShader() {
  std::vector<char> buffer;
  std::ifstream file(m_Path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error(
        "Shader2::CreateShader::ERROR: Failed to open shader file with path " +
        m_Path);
  }

  buffer.resize(static_cast<size_t>(file.tellg()));
  file.seekg(0, std::ios::beg);
  file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
  file.close();

  vk::ShaderModuleCreateInfo createInfo{
      .codeSize = buffer.size(),
      .pCode = reinterpret_cast<const uint32_t *>(buffer.data())};
  m_ShaderModule = m_pContext->device.createShaderModule(createInfo);
}
} // namespace Core
} // namespace SimpleEngine
