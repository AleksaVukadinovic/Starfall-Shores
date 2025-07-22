//

#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>

#include <format>
#include <spdlog/spdlog.h>

namespace engine::util {

std::string_view EngineError::type_string(Type error) {
    switch (error) {
        case Type::Unimplemented: return "Unimplemented";
        case Type::ShouldNotReachHere: return "ShouldNotReachHere";
        case Type::GuaranteeViolation: return "GuaranteeViolation";
        case Type::FileNotFound: return "FileNotFound";
        case Type::ConfigurationError: return "ConfigurationError";
        case Type::ShaderCompilationError: return "ShaderCompilationError";
        case Type::OpenGLError: return "OpenGLError";
        case Type::AssetLoadingError: return "AssetLoadingError";
        default: return "Unknown";
    }
}

std::string EngineError::report() const {
    return std::format("{}: in {}:{}.\n Message: {}", type_string(m_error), location().file_name(),
                       location().line(), message());
}

} // namespace engine
