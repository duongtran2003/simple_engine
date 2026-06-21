#pragma once

#include "core/input/key_code.hpp"
#include <GLFW/glfw3.h>
#include <array>

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

  GLFWwindow *window;
  std::array<bool, 512> keys{false};
  std::array<KeyState, 512> keyStates{KeyState::None};

public:
  Input() = delete;
  Input(GLFWwindow *window);

  bool isKeyJustPressed(Key key) const;
  bool isKeyHeld(Key key) const;
  bool isKeyJustReleased(Key key) const;

  void update();
};
} // namespace Core
} // namespace SimpleEngine
