#include "core/camera.hpp"
#include "core/component/camera_component.hpp"
#include "core/entity/entity.hpp"

namespace SimpleEngine {
namespace Core {
Camera::Camera() : Entity("g_camera") { addComponent<CameraComponent>(); }
} // namespace Core
} // namespace SimpleEngine
