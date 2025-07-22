/**
 * @file Configuration.hpp
 * @brief Defines the Configuration class that serves as the interface for configuration management.
*/

#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <json.hpp>

namespace engine::util {
/**
* @class Configuration
* @brief Loads and stores the configuration file.
*
* The configuration file is loaded from the default path: @ref CONFIG_FILE_NAME=config.json.
* The configuration file is stored in a static variable, so it is shared between all instances of the @ref Configuration class.
*/
class Configuration {
    friend class App;

public:
    using json = nlohmann::json;

    /**
    * @brief Get the configuration file.
    * @returns The configuration file.
    */
    static json &config();

    /**
    * @brief Get the instance of the @ref Configuration class.
    * @returns The instance of the @ref Configuration class.
    */
    static Configuration *instance();

    /**
    * @brief Initialize the @ref Configuration class.
    */
    void initialize();

private:
    /**
    * @brief Get the path to the configuration file.
    * @returns The path to the configuration file.
    */
    static std::filesystem::path get_config_path();

    /**
    * @brief Create the default configuration file.
    * @returns The default configuration file.
    */
    static json create_default();

    /**
    * @brief The default name of the configuration file.
    */
    constexpr static std::string_view CONFIG_FILE_NAME = "config.json";

    /**
    * @brief The configuration file.
    */
    json m_config;
};
}
#endif //CONFIGURATION_HPP
