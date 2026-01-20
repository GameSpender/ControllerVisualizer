#pragma once
#include <unordered_map>
#include "GLFW/glfw3.h"    

enum class Action {
    MoveHorizontal,
    MoveVertical,
    MousePositionHorizontal,
    MousePositionVertical,
    AimHorizontal,
    AimVertical,
    Aim,
    PrimaryAbility,
    SecondaryAbility,
	TertiaryAbility,
	QuaternaryAbility,
    Dash,

};

enum class DeviceType { Keyboard, Mouse, Gamepad };

struct InputDevice {
    DeviceType type = DeviceType::Keyboard;
    int id = 0;

    bool operator==(const InputDevice& other) const {
        return type == other.type && id == other.id;
    }
};


// Needed for using InputDevice in unordered containers
namespace std {
    template<>
    struct hash<InputDevice> {
        std::size_t operator()(const InputDevice& d) const noexcept {
            return (std::hash<int>()(static_cast<int>(d.type)) << 1) ^ std::hash<int>()(d.id);
        }
    };
}

struct RawInputData {
    bool keyDown[GLFW_KEY_LAST] = {};
    bool mouseDown[8] = {};
    double mouseX = 0, mouseY = 0;
    float gamepadAxes[4][GLFW_GAMEPAD_AXIS_LAST] = {};
    unsigned char gamepadButtons[4][GLFW_GAMEPAD_BUTTON_LAST] = {};
};


struct InputBinding {
    InputDevice device;
    int code = -1;       // key/button
    int axisCode = -1;   // for analog axes
    float scale = 1.0f;
    float offset = 0.0f; 
    float deadzone = 0.0f; // ignore inputs below this magnitude

    // digital match (buttons/keys)
    bool matchesDigital(const RawInputData& raw) const {
        switch (device.type) {
        case DeviceType::Keyboard:
            return code >= 0 && raw.keyDown[code];
        case DeviceType::Mouse:
            return code >= 0 && raw.mouseDown[code];
        case DeviceType::Gamepad:
            if (device.id < 0 || device.id >= 4) return false;
            if (axisCode >= 0) {
                float value = raw.gamepadAxes[device.id][axisCode];
                if (std::abs(value) < deadzone) return false; // apply deadzone
                value = value * scale + offset;
                return std::abs(value) > 0.5f;
            }
            return code >= 0 && raw.gamepadButtons[device.id][code];
        default:
            return false;
        }
    }

    // analog value (axes, mouse delta, etc.)
    float getAnalogValue(const RawInputData& raw) const {
        float value = 0.0f;

        switch (device.type) {
        case DeviceType::Keyboard:
            value = (code >= 0 && raw.keyDown[code]) ? scale : 0.0f;
            break;
        case DeviceType::Mouse:
            if (axisCode == 0) value = static_cast<float>(raw.mouseX) * scale;
            else if (axisCode == 1) value = static_cast<float>(raw.mouseY) * scale;
            break;
        case DeviceType::Gamepad:
            if (device.id < 0 || device.id >= 4) return 0.0f;
            if (axisCode >= 0) {
                value = raw.gamepadAxes[device.id][axisCode];
                if (std::abs(value) < deadzone) value = 0.0f; // apply deadzone
                value = value * scale;
            }
            break;
        default:
            return 0.0f;
        }

        return value + offset;
    }
};




class PlayerInput {
public:
    std::vector<InputDevice> devices;  // devices assigned to this player

    // bindings: each action can have multiple bindings
    std::unordered_map<Action, std::vector<InputBinding>> bindings;

    // computed every frame
    std::unordered_map<Action, bool> actionState;        // current frame
    std::unordered_map<Action, bool> prevActionState;    // previous frame
    std::unordered_map<Action, float> analogState;       // analog value for axes

    void update(const RawInputData& raw) {
        prevActionState = actionState;

        for (auto& [action, bindingList] : bindings) {
            bool active = false;
            float analogValue = 0.0f;

            for (auto& binding : bindingList) {
                if (!playerHasDevice(binding.device)) continue;

                if (binding.matchesDigital(raw)) {
                    active = true;
                }
                analogValue += binding.getAnalogValue(raw);
            }

            actionState[action] = active;
            analogState[action] = analogValue; // could be >1.0 if multiple sources; normalize if needed
        }
    }

    // Digital queries
    bool isDown(Action action) const {
        auto it = actionState.find(action);
        return it != actionState.end() && it->second;
    }

    bool isPressed(Action action) const {
        return isDown(action) && !prevIsDown(action);
    }

    bool isReleased(Action action) const {
        return !isDown(action) && prevIsDown(action);
    }

    // Analog query
    float getAnalog(Action action) const {
        auto it = analogState.find(action);
        return it != analogState.end() ? it->second : 0.0f;
    }

    vec2 getPosition(Action actionHorizontal, Action actionVertical) const {
        return vec2(getAnalog(actionHorizontal), getAnalog(actionVertical));
    }

private:
    bool prevIsDown(Action action) const {
        auto it = prevActionState.find(action);
        return it != prevActionState.end() && it->second;
    }

    bool playerHasDevice(const InputDevice& device) const {
        for (const auto& d : devices) {
            if (d == device) return true;
        }
        return false;
    }
};


class InputSystem {
public:
    std::vector<InputDevice> devices;
    std::vector<PlayerInput> players;

    RawInputData raw; // populated by polling GLFW

    void update() {
        pollRawInput(); // fill raw with current input state

        for (PlayerInput& player : players)
            player.update(raw);
    }

private:
    void pollRawInput() {
        // keyboard
        for (int key = 0; key < GLFW_KEY_LAST; ++key)
            raw.keyDown[key] = glfwGetKey(glfwGetCurrentContext(), key) == GLFW_PRESS;

        // mouse buttons
        for (int i = 0; i < 8; ++i)
            raw.mouseDown[i] = glfwGetMouseButton(glfwGetCurrentContext(), i) == GLFW_PRESS;

        // mouse position
        glfwGetCursorPos(glfwGetCurrentContext(), &raw.mouseX, &raw.mouseY);

        // gamepads
        for (int i = 0; i < 4; ++i) {
            if (glfwJoystickIsGamepad(i)) {
                GLFWgamepadstate state;
                if (glfwGetGamepadState(i, &state)) {
                    for (int a = 0; a < GLFW_GAMEPAD_AXIS_LAST; ++a)
                        raw.gamepadAxes[i][a] = state.axes[a];
                    for (int b = 0; b < GLFW_GAMEPAD_BUTTON_LAST; ++b)
                        raw.gamepadButtons[i][b] = state.buttons[b];
                }
            }
        }
    }
};