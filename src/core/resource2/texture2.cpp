#include "core/resource2/texture2.hpp"
#include "core/image/image_handle.hpp"
#include "core/image/image_sampler.hpp"
#include <cstdint>
#include <string>

namespace SimpleEngine {
namespace Core {
Texture2::Texture2(const std::string &path) { m_Path = path; }
Texture2::~Texture2() {
  // TODO: Destructor
}

const std::string &Texture2::GetPath() const { return m_Path; }
ImageHandle Texture2::GetImageHandle() const { return m_ImageHandle; }
const ImageSampler *Texture2::GetSampler() const { return m_Sampler; }
uint32_t Texture2::GetBindlessIndex() const { return m_BindlessIndex; }

void Texture2::SetSampler(ImageSampler *sampler) { m_Sampler = sampler; }
void Texture2::SetBindlessIndex(uint32_t index) { m_BindlessIndex = index; }

void Texture2::LoadTexture() {

}

} // namespace Core
} // namespace SimpleEngine
