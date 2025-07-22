/**
 * @file Errors.hpp
 * @brief Defines  Error, UserError, and EngineError classes, along with macros for error handling.
*/

#ifndef MATF_RG_PROJECT_ERRORS_HPP
#define MATF_RG_PROJECT_ERRORS_HPP

#include <source_location>
#include <string>
#include <utility>
#include <format>

namespace engine::util {
/**
* @class Error
* @brief Base class for all errors.
*
* Exceptions are not meant to be caught. They are meant to be used as a graceful way to stop the program and give
* a meaningful instruction to the user on how to fix the problem.
*/
class Error : public std::exception {
public:
    /**
    * @brief Construct an Error.
    * @param message The error message.
    * @param location The location of the error. This is used to get the file, line and function name where the error occurred.
    */
    explicit Error(std::string message, std::source_location location = std::source_location::current())
            : m_message(std::move(message))
              , m_location(location) {
    }

    /**
    * @brief Get the error message.
    * @returns The error message.
    */
    const std::string &message() const {
        return m_message;
    }

    /**
    * @brief Get the location of the error.
    * @returns The location of the error.
    */
    std::source_location location() const {
        return m_location;
    }

    /**
    * @brief Get the error report. You can override this method in derived classes to provide a more detailed error report.
    * @returns The error report.
    */
    virtual std::string report() const {
        return m_message;
    };

private:
    std::string m_message;
    std::source_location m_location;
};

/**
* @class EngineError
* @brief Represents an error that occurred in the engine.
*/
class EngineError final : public Error {
public:
    /**
    * @brief The type of the engine error.
    */
    enum class Type {
        /**
        * @brief The error that occurs when a function is not implemented. Use this during development to mark a function as not implemented yet.
        * It is not meant to be thrown in production.
        */
        Unimplemented,
        /**
        * @brief The error that occurs when a function should not reach a certain point.
        * Use this to mark a path that should not be reached. If it is reached, it means that the program is in an inconsistent state.
        */
        ShouldNotReachHere,
        /**
        * @brief The error that occurs when a guarantee is violated.
        */
        GuaranteeViolation,
        /**
        * @brief The error that occurs when a file is not found.
        */
        FileNotFound,
        /**
        * @brief The error that occurs when a configuration is invalid.
        */
        ConfigurationError,
        /**
        * @brief The error that occurs when a shader compilation fails.
        */
        ShaderCompilationError,
        /**
        * @brief The error that occurs when an OpenGL error occurs.
        */
        OpenGLError,
        /**
        * @brief The error that occurs when an asset loading fails.
        */
        AssetLoadingError,

        EngineErrorCount
    };

    /**
    * @brief Get the string representation of the engine error type.
    * @param error The engine error type.
    * @returns The string representation of the engine error type.
    */
    static std::string_view type_string(Type error);

    /**
    * @brief Construct an EngineError.
    * @param error_type The type of the engine error.
    * @param message The error message.
    * @param location The location of the error. This is used to get the file, line and function name where the error occurred.
    */
    EngineError(Type error_type, std::string message,
                std::source_location location = std::source_location::current())
            : Error(std::move(message), std::move(location))
              , m_error(error_type) {
    }

    /**
    * @brief Get the error report.
    * @returns The error report.
    */
    std::string report() const override;

private:
    Type m_error;
};

/**
* @class UserError
* @brief Represents an error that occurred in the user's code.
*
* This is meant to be used to report errors that occur in the user's code.
* It is not meant to be used to report errors that occur in the engine.
*
* Exceptions are not meant to be caught. They are meant to be used as a graceful way to stop the program and give
* a meaningful instruction to the user on how to fix the problem.
*
* Inherit from this class to create your own errors.
*/
class UserError : public Error {
public:
    using Error::Error;
};
} // namespace engine

/**
* @brief Guarantees that an expression is true. If it is not, an @ref engine::util::EngineError is thrown.  
* @param expr The expression to guarantee.
* @param msg The error message.
* @param ... The arguments to the error message.
*/
#define RG_GUARANTEE(expr, msg, ...)                                                                                   \
    do {                                                                                                               \
        if (!(expr)) {                                                                                                 \
            throw engine::util::EngineError(engine::util::EngineError::Type::GuaranteeViolation, std::format(msg, ##__VA_ARGS__), std::source_location::current());            \
        }                                                                                                              \
    } while (0)

/**
* @brief Guarantees that a path should not be reached. If it is reached, an @ref engine::util::EngineError is thrown.
* @param msg The error message.
* @param ... The arguments to the error message.
*/
#define RG_SHOULD_NOT_REACH_HERE(msg, ...)                                                                             \
    do {                                                                                                               \
        throw engine::util::EngineError(engine::util::EngineError::Type::ShouldNotReachHere, std::format(msg, ##__VA_ARGS__), std::source_location::current());                \
    } while (0)

/**
* @brief Path is not implemented. If it is implemented, an @ref engine::util::EngineError is thrown.
* @param msg The error message.
* @param ... The arguments to the error message.
*/
#define RG_UNIMPLEMENTED(msg, ...)                                                                                     \
    do {                                                                                                               \
        throw engine::util::EngineError(engine::util::EngineError::Type::Unimplemented, std::format(msg, ##__VA_ARGS__), std::source_location::current());                     \
    } while (0)

/**
* @brief Throws an @ref engine::util::EngineError.
* @param type The type of the engine error.
* @param msg The error message.
* @param ... The arguments to the error message.
*/
#define RG_ENGINE_ERROR(type, msg, ...)                                                                               \
    do {                                                                                                               \
        throw engine::util::EngineError(type, std::format(msg, ##__VA_ARGS__), std::source_location::current()); \
    } while(0)

#endif//MATF_RG_PROJECT_ERRORS_HPP
