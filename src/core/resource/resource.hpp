#pragma once

#include "core/render_context.hpp"
#include <string>
namespace SimpleEngine {
namespace Core {
class Resource {
private:
  std::string resourceId;
  bool loaded = false;

public:
  Resource(const std::string &id, const RenderContext &renderContext);
  virtual ~Resource() = default;

  const std::string &getId() const;
  const RenderContext &getRenderContext() const;

  bool isLoaded() const;

  bool load();
  void unload();

protected:
  virtual bool doLoad() = 0;
  virtual void doUnload() = 0;

  const RenderContext &renderContext;
};
} // namespace Core
} // namespace SimpleEngine
