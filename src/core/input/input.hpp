#pragma once

#include "core/input/key_code.hpp"
#include "core/render_context.hpp"
#include <GLFW/glfw3.h>
#include <array>
#include <glm/ext/vector_float2.hpp>

namespace SimpleEngine {
namespace Core {

class Input {
public:
  enum class KeyState {
    None,
    JustPressed,
    Held,
    JustReleased,
  };

private:
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);
  static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

  const RenderContext &context;
  std::array<bool, 512> keys{false};
  std::array<KeyState, 512> keyStates{KeyState::None};

  bool mouseLocked = false;
  glm::vec2 lastMousePos = {0.0f, 0.0f};
  glm::vec2 mouseDelta = {0.0f, 0.0f};
  bool mouseFirstEnter = true;

public:
  Input() = delete;
  Input(const RenderContext &context);

  bool isKeyJustPressed(Key key) const;
  bool isKeyHeld(Key key) const;
  bool isKeyJustReleased(Key key) const;
  bool isMouseLocked() const;
  glm::vec2 getMouseDelta() const;
  void clearMouseDelta();

  void toggleMouseLock();

  void handleMouseMove(double xpos, double ypos);
  void update();
};
} // namespace Core
} // namespace SimpleEngine
