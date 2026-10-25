#pragma once

#include "core/render_context.hpp"
#include "vulkan/vulkan.hpp"
#include <string>
namespace SimpleEngine {
namespace Core {

class Shader2 {
public:
  Shader2() = default;
  Shader2(const std::string &path, vk::ShaderStageFlags shaderStage, const RenderContext* pRenderContext);
  ~Shader2();

  const std::string &GetShaderPath() const;
  vk::ShaderStageFlags GetShaderStage() const;
  vk::ShaderModule GetShaderModule() const;
  bool IsReady() const;

private:
  void CreateShader();

  const RenderContext *m_pContext;

  std::string m_Path;
  vk::ShaderModule m_ShaderModule = nullptr;
  vk::ShaderStageFlags m_ShaderStage;
};
} // namespace Core
} // namespace SimpleEngine
