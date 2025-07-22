/**
 * @file ArgParser.hpp
 * @brief Defines the ArgParser class that serves as the interface for argument parsing.
*/

#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

#include <string_view>
#include <optional>
#include <string>

namespace engine::util {
/**
* @class ArgParser
* @brief Parses command line arguments.
*/
class ArgParser {
    friend class App;

public:
    static ArgParser *instance();

    /**
    * @brief Get the value of an argument.
    * Supported types: bool, int, long long, float, double, std::string.
    * @tparam T The type of the argument.
    * @param name The name of the argument.
    * @param default_value The default value of the argument.
    * @returns The value of the argument.
    */
    template<typename T>
    std::optional<T> arg(std::string_view name, std::optional<T> default_value = {}) {
        std::string arg_value = get_arg_value(name);
        if (arg_value.empty()) {
            return default_value.has_value() ? default_value.value() : T{};
        }
        std::size_t parsed = 0;
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
            return std::stoi(arg_value, &parsed);
        } else if constexpr (std::is_same_v<T, long long> || std::is_same_v<T, int64_t>) {
            return std::stoll(arg_value, &parsed);
        } else if constexpr (std::is_same_v<T, float>) {
            return std::stof(arg_value, &parsed);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(arg_value, &parsed);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg_value;
        } else {
            static_assert(false, "This type is not supported!");
        }
    }

    /**
    * @brief Initialize the ArgParser with the command line arguments.
    * @param argc The number of command line arguments.
    * @param argv The command line arguments.
    */
    void initialize(int argc, char **argv);

private:
    /**
    * @brief Get the string value of an argument.
    * @param arg_name The name of the argument.
    * @returns The string value of the argument.
    */
    std::string get_arg_value(std::string_view arg_name);

    ArgParser() = default;

    int m_argc = 0;
    char **m_argv = nullptr;
};
}
#endif //ARGPARSER_HPP
