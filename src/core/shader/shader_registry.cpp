#include "core/shader/shader_registry.hpp"
#include "core/render_context.hpp"
#include "core/shader/shader2.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
#include <string>
#include <utility>

namespace SimpleEngine {
namespace Core {
ShaderRegistry::ShaderRegistry(const RenderContext *pRenderContext) {
  m_pContext = pRenderContext;
}

ShaderRegistry::~ShaderRegistry() { m_ShadersMap.clear(); }

Shader2 *ShaderRegistry::LoadShader(const std::string &path,
                                    vk::ShaderStageFlags shaderStage) {
  auto shader = std::make_unique<Shader2>(path, shaderStage, m_pContext);
  if (shader->IsReady()) {
    m_ShadersMap[path] = std::move(shader);
    return m_ShadersMap[path].get();
  }

  return nullptr;
}

Shader2 *ShaderRegistry::GetShader(const std::string &path) {
  auto it = m_ShadersMap.find(path);
  if (it != m_ShadersMap.end()) {
    return it->second.get();
  }

  return nullptr;
}
} // namespace Core
} // namespace SimpleEngine
