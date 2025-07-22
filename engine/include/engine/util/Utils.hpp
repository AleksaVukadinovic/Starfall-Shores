/**
 * @file Utils.hpp
 * @brief Defines utility functions, macros, algorithms, and data structures.
 */

#ifndef MATF_RG_PROJECT_UTILS_HPP
#define MATF_RG_PROJECT_UTILS_HPP

#include <format>
#include <source_location>
#include <vector>
#include <mutex>
#include <filesystem>
#include <functional>
#include <unordered_set>
#include <type_traits>

/// @cond
template<class... Ts>
struct overloaded : Ts ... {
    using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template<typename Func>
struct DeferImpl {
    DeferImpl(Func f) : f(f) {
    }

    ~DeferImpl() {
        f();
    }

    Func f;
};

struct MakeDeferImpl {
};

template<typename Func>
DeferImpl<Func> operator<<(MakeDeferImpl, Func f) {
    return DeferImpl(f);
}

#define STR(a) #a
#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
/// @endcond

/**
* @brief Creates a defer statement.
* @param f The function to defer.
* 
* @code 
*   { 
*       unsigned char * image = stbi_load("image.png", &width, &height, &channels, 0);   
*       defer { stbi_image_free(image); };
*       // use image
*   } <-- defer statement is executed here
* @endcode
*/
#define defer auto CONCAT(defer_stmt_, __LINE__) = MakeDeferImpl() << [&]

/**
* @brief Shorthand for std::begin(container), std::end(container).
 */
#define range(container) std::begin(container), std::end(container)

/**
* @brief Shorthand for std::cbegin(container), std::cend(container).
 */
#define crange(container) std::cbegin(container), std::cend(container)

namespace engine::util {
/**
* @brief Turns on tracing.
*/
void tracing_on();

/**
* @brief Turns off tracing.
*/
void tracing_off();

/**
* @brief Traces a function call.
* @param location The location of the trace. Defaults to the call site location.
* @code
* void foo() {
*   trace();
* }
* @endcode
* Prints: "foo() at foo.cpp:10"
*/
void trace(std::source_location location = std::source_location::current());

/**
* @brief Reads a text file.
* @param path The path to the file.
* @returns The content of the file.
*/
std::string read_text_file(const std::filesystem::path &path);

/**
* @brief Calls an action once.
* @param action The action to call.
* @code
* void foo() {
*   once([] { std::cout << "foo\n"; });
* }
* @endcode
* Prints: "foo" once when the control flow reaches the function for the first time.
* If the function foo is called again, the action is not called again.
*/
template<typename Action>
void once(Action action) {
    static std::once_flag once;
    std::call_once(once, action);
};

/**
* @brief Contains algorithm.
*/
namespace alg {
/**
* @brief Checks if a container contains a value. Shorthand for std::find.
* @param container The container.
* @param value The value to check.
* @returns True if the container contains the value, false otherwise.
*/
template<typename Container, typename T>
bool contains(const Container &container, const T &value) {
    if constexpr (requires { container.contains(value); }) {
        return container.contains(value);
    } else {
        return std::find(std::cbegin(container), std::cend(container), value) != std::cend(container);
    }
}

/**
* @brief Topologically sorts a directed acyclic graph represented by a range and an adjacent function. Make sure that the graph is acyclic before calling this function.
* Use @ref has_cycle to check for cycles.
* @param first The first element of the range.
* @param last The last element of the range.
* @param adjacent The function that returns the adjacent elements for a given element. In a directed graph, this is the function that returns the successors of a given node.
*/
template<typename It, typename Adjacent>
void topological_sort(It first, It last, Adjacent adjacent) {
    using ElementType = std::remove_reference_t<decltype(*first)>;
    std::unordered_set<ElementType> visited;
    std::vector<ElementType> stack;

    auto visit = [&](auto &self, ElementType &current) mutable -> void {
        visited.emplace(current);
        for (auto next: adjacent(current)) {
            if (!visited.contains(next)) {
                self(self, next);
            }
        }
        stack.push_back(current);
    };


    auto it = first;
    while (it != last) {
        if (!visited.contains(*it)) {
            visit(visit, *it);
        }
        ++it;
    }
    std::move(stack.rbegin(), stack.rend(), first);
}

/**
* @brief Checks if a graph represented by a range and an adjacent function has a cycle.
* @param first The first element of the range.
* @param last The last element of the range.
* @param adjacent The function that returns the adjacent elements for a given element. In a directed graph, this is the function that returns the successors of a given node.
* @param cycle_output The output iterator where the cycle is written. If not provided, the function returns true if a cycle is found, false otherwise.
* @returns True if a cycle is found, false otherwise.
*/
template<typename It, typename Adjacent, typename OutputIt = std::nullptr_t>
bool has_cycle(It first, It last, Adjacent adjacent, OutputIt cycle_output = nullptr) {
    using ElementType = std::remove_reference_t<decltype(*first)>;
    std::unordered_set<ElementType> visited;
    std::unordered_set<ElementType> path;
    auto visit = [&](auto &self, ElementType &current) mutable -> bool {
        visited.emplace(current);
        path.emplace(current);
        for (ElementType &next: adjacent(current)) {
            if (!visited.contains(next) && self(self, next)) {
                return true;
            }
            if (path.contains(next)) {
                return true;
            }
        }
        path.erase(current);
        return false;
    };

    for (auto root = first; root != last; ++root) {
        if (!visited.contains(*root) && visit(visit, *root)) {
            if constexpr (!std::is_same_v<OutputIt, std::nullptr_t>) {
                std::move(path.begin(), path.end(), cycle_output);
            };
            return true;
        }
    }
    return false;
}
} // namespace alg

namespace ds {
}
} // namespace engine

#endif//MATF_RG_PROJECT_UTILS_HPP
