/**
 * @file Input.hpp
 * @brief Defines the Input class that serves as the interface handling user input via keyboard and mouse.
 */

#ifndef INPUT_HPP
#define INPUT_HPP

#include <string_view>

namespace engine::platform {
/**
* @brief All the Keys that the engine can register.
*/
enum KeyId {
    MOUSE_BUTTON_1 = 0,
    MOUSE_BUTTON_2,
    MOUSE_BUTTON_3,
    MOUSE_BUTTON_4,
    MOUSE_BUTTON_5,
    MOUSE_BUTTON_6,
    MOUSE_BUTTON_7,
    MOUSE_BUTTON_8,
    MOUSE_BUTTON_LAST,
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    KEY_SPACE,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_EQUAL,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFT_BRACKET,
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_GRAVE_ACCENT,
    KEY_WORLD_1,
    KEY_WORLD_2,
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,
    KEY_COUNT,
};

/**
* @class Key
* @brief Represents the state of the key in a given frame.
*/
class Key final {
    friend class PlatformController;

public:
    /**
    * @brief The state of the key with regard to whether it is pressed or not.
    */
    enum class State {
        /**
        * @brief The key is up.
        */
        Released,
        /**
        * @brief The key has just been pressed in the current frame.
        * This state only lasts for the frame in which the key was first registered as pressed.
        */
        JustPressed,
        /**
        * @brief The key is being held down.
        * If the key was `JustPressed` in the previous frame, and it is still down in the current frame,
        * its state transitions into Pressed.
        */
        Pressed,
        /**
        * @brief Lasts only for the frame in which the key was released.
        * In the next frame, if the key is not pressed again, it transitions into Released.
        */
        JustReleased
    };

    /**
    * @returns The state of the key in the current frame.
    */
    State &state() {
        return m_state;
    }

    /**
    * @returns The id of the key.
    */
    KeyId id() const {
        return m_key;
    }

    /**
    * @returns The @ref KeyId as a string.
    */
    std::string_view name();

    State state() const {
        return m_state;
    }

    /**
    *  @returns The @ref State of the key as a string_view.
    */
    std::string_view state_str() const;

    /**
    * @brief Shorthand method for checking whether the key is down.
    * @returns True if the key is Pressed or JustPressed
    */
    bool is_down() const {
        return (m_state == State::Pressed || m_state == State::JustPressed);
    }

    /**
    * @brief Shorthand method for checking whether the key is up.
    * @returns True if the key is Released or JustReleased
    */
    bool is_up() const {
        return (m_state == State::Released || m_state == State::JustReleased);
    }

private:
    KeyId m_key = KEY_COUNT;
    State m_state = State::Released;
};

/**
*  @struct MousePosition
*  @brief Represents mouse position in a given frame relative to the top left corner of the screen.
*/
struct MousePosition {
    /**
    * @brief X coordinate of the mouse position relative to the top left corner of the screen.
    */
    float x;
    /**
    * @brief Y coordinate of the mouse position relative to the top left corner of the screen.
    */
    float y;
    /**
    * @brief The change in the X coordinate from the previous frame.
    */
    float dx;
    /**
    * @brief The change in the Y coordinate from the previous frame.
    */
    float dy;

    /**
    * @brief The rotation of the scroll button.
    */
    float scroll;
};
}
#endif //INPUT_HPP
