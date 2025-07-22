/**
 * @file Window.hpp
 * @brief Defines the Window class  that provides basic window properties.
*/

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <string>

struct GLFWwindow;

namespace engine::platform {
/**
* @brief Holds window properties.
*/
class Window final {
    friend class PlatformController;

public:
    /**
    * @brief Get the height of the window.
    */
    int height() const {
        return m_height;
    }

    /**
    * @brief Get the width of the window.
    */
    int width() const {
        return m_width;
    }

    /**
    * @brief Get the title of the window.
    */
    const std::string &title() const {
        return m_title;
    }

    /**
    * @brief Get the window handle pointer. You are not supposed to use this value outside the `engine` namespace.
    * @returns An opaque pointer to the GLFWwindow. You are not supposed to use this return value
    * outside the Engine.
    */
    GLFWwindow *handle_() const {
        return m_handle;
    }

private:
    GLFWwindow *m_handle{};
    int m_width{};
    int m_height{};
    std::string m_title{};

    Window() = default;

    Window(GLFWwindow *handle, int width, int height, std::string title) : m_handle(handle)
                                                                           , m_width(width)
                                                                           , m_height(height)
                                                                           , m_title(std::move(title)) {
    }
};
}
#endif //WINDOW_HPP
