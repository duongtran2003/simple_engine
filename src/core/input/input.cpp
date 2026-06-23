#include "core/input/input.hpp"
#include "core/input/key_code.hpp"
#include "core/render_context.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/vector_float2.hpp>

namespace SimpleEngine {
namespace Core {
Input::Input(const RenderContext &context) : context(context) {
  glfwSetWindowUserPointer(context.window, this);
  glfwSetKeyCallback(context.window, keyCallback);
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

void Input::toggleMouseLock() {
  mouseLocked = !mouseLocked;
  if (mouseLocked) {
    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    mouseFirstEnter = true;
  } else {
    glfwSetInputMode(context.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

  if (mouseLocked) {
    double mouseX, mouseY;
    glfwGetCursorPos(context.window, &mouseX, &mouseY);

    if (mouseFirstEnter) {
      lastMousePos = {mouseX, mouseY};
      mouseFirstEnter = false;
    }

    mouseDelta = {mouseX - lastMousePos.x, lastMousePos.y - mouseY};

    lastMousePos = {mouseX, mouseY};
  } else {
    mouseDelta = {0.0f, 0.0f};
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

bool Input::isMouseLocked() const { return mouseLocked; }

glm::vec2 Input::getMouseDelta() const { return mouseDelta; }

void Input::clearMouseDelta() { mouseDelta = {0.0f, 0.0f}; }
} // namespace Core
} // namespace SimpleEngine
