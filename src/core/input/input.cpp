#include "core/input/input.hpp"
#include "core/input/key_code.hpp"
#include <GLFW/glfw3.h>

namespace SimpleEngine {
namespace Core {
Input::Input(GLFWwindow *window) : window(window) {
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, keyCallback);
};

void Input::keyCallback(GLFWwindow *window, int key, int scancode, int action,
                        int mods) {
  auto *instance = static_cast<Input *>(glfwGetWindowUserPointer(window));
  if (!instance || key < 0 || key >= 512) {
    return;
  }

  if (action == GLFW_PRESS) {
    instance->keys[key] = true;
  } else if (action == GLFW_RELEASE) {
    instance->keys[key] = false;
  }
}

void Input::update() {
  for (int i = 0; i < 512; i++) {
    bool isPressed = keys[i];
    KeyState lastState = keyStates[i];

    if (isPressed) {
      if (lastState == KeyState::None || lastState == KeyState::JustReleased) {
        keyStates[i] = KeyState::JustPressed;
      } else if (lastState == KeyState::JustPressed) {
        keyStates[i] = KeyState::Held;
      }
    } else {
      if (lastState == KeyState::JustPressed || lastState == KeyState::Held) {
        keyStates[i] = KeyState::JustReleased;
      } else if (lastState == KeyState::JustReleased) {
        keyStates[i] = KeyState::None;
      }
    }
  }
}

bool Input::isKeyJustPressed(Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::JustPressed;
}

bool Input::isKeyHeld(Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::Held ||
         keyStates[keyCode] == KeyState::JustPressed;
}

bool Input::isKeyJustReleased(Key key) const {
  int keyCode = static_cast<int>(key);
  return keyStates[keyCode] == KeyState::JustReleased;
}
} // namespace Core
} // namespace SimpleEngine
