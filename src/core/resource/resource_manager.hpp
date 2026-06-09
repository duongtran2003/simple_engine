#include "core/resource/resource.hpp"
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace SimpleEngine {
namespace Core {
template <typename T> class ResourceHandle;

class ResourceManager {
private:
  struct ResourceData {
    std::shared_ptr<Resource> resource;
    int refCount;
  };
  std::unordered_map<std::type_index,
                     std::unordered_map<std::string, ResourceData>>
      resources;

public:
  template <typename T> ResourceHandle<T> load(const std::string &resourceId);
  template <typename T> void release(const std::string &resourceId);
  template <typename T> T *getResource(const std::string &resourceId);
  template <typename T> bool hasResource(const std::string &resourceId);
};
} // namespace Core
} // namespace SimpleEngine
