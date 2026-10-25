#pragma once

#include "core/render_context.hpp"
#include "core/shader/shader2.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace SimpleEngine {
namespace Core {
class ShaderRegistry {
public:
  ShaderRegistry() = default;
  ShaderRegistry(const RenderContext *pRenderContext);
  ~ShaderRegistry();

  Shader2 *LoadShader(const std::string &path,
                      vk::ShaderStageFlags shaderStage);
  Shader2 *GetShader(const std::string &path);

private:
  std::unordered_map<std::string, std::unique_ptr<Shader2>> m_ShadersMap;

  const RenderContext *m_pContext;
};
} // namespace Core
} // namespace SimpleEngine
