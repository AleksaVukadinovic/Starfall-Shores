
#include <engine/util/Utils.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <engine/util/Configuration.hpp>
#include <engine/util/ArgParser.hpp>

namespace engine::util {
static bool g_tracing = true;

void tracing_on() {
    g_tracing = true;
}

void tracing_off() {
    g_tracing = false;
}

void trace(std::source_location location) {
    if (g_tracing) {
        spdlog::info("{}, in {}:{}", location.function_name(), location.file_name(), location.line());
    }
}

void Configuration::initialize() {
    auto config_path = get_config_path();
    std::ifstream f(config_path);
    if (!f.is_open()) {
        throw EngineError(EngineError::Type::FileNotFound,
                          std::format("Failed to load configuration file {}", config_path.string()));
    }

    try {
        m_config = json::parse(f);
    } catch (const std::exception &e) {
        std::string message(e.what());
        throw EngineError(EngineError::Type::ConfigurationError, std::format(
                "Error \"{}\" occurred while parsing the configuration file. "
                "Please make sure that the file is in the correct json format.",
                message));
    }
    spdlog::info("Configuration initialized.");
}

std::filesystem::path Configuration::get_config_path() {
    auto config_arg = ArgParser::instance()->arg<std::string>("--configuration", "config.json");
    if (!config_arg.has_value() || !exists(std::filesystem::path(config_arg.value()))) {
        std::ofstream f(CONFIG_FILE_NAME.data());
        if (!f.is_open()) {
            throw EngineError(EngineError::Type::ConfigurationError,
                              std::format("Failed to open configuration file."));
        }
        auto config = create_default();
        f << config.dump(4);
        return CONFIG_FILE_NAME;
    }
    return config_arg.value();
}

nlohmann::json Configuration::create_default() {
    json default_config;
    default_config["window"]["width"] = 800;
    default_config["window"]["height"] = 600;
    default_config["window"]["title"] = "Hello, window!";
    return default_config;
}

Configuration *Configuration::instance() {
    static Configuration configuration;
    return &configuration;
}

Configuration::json &Configuration::config() {
    return instance()->m_config;
}

ArgParser *ArgParser::instance() {
    static ArgParser arg_parser;
    return &arg_parser;
}

void ArgParser::initialize(int argc, char **argv) {
    m_argc = argc;
    m_argv = argv;
    spdlog::info("ArgParser initialized.");
}

std::string ArgParser::get_arg_value(std::string_view arg_name) {
    for (int i = 0; i < m_argc; ++i) {
        std::string_view token(m_argv[i]);
        if (token == arg_name) {
            std::string arg_value(m_argv[i + 1]);
            RG_GUARANTEE(!arg_value.starts_with("--"), "No get_arg_value for argument: \"{}\" provided.", arg_name);
            return arg_value;
        }
    }
    return "";
}

std::string read_text_file(const std::filesystem::path &path) {
    RG_GUARANTEE(std::filesystem::exists(path), "File {} doesn't exist.", path.string());
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
} // namespace engine
