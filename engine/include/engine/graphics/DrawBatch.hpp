/**
 * @file DrawBatch.hpp
 * @brief A model placed at a fixed set of transforms, prepared once for fast drawing and culling.
 */

#pragma once

#include <engine/resources/InstanceBuffer.hpp>
#include <engine/util/Geometry.hpp>
#include <engine/util/Transform.hpp>
#include <optional>
#include <span>
#include <vector>

namespace engine::resources {
class Model;
}

namespace engine::graphics {

/**
 * @brief Vertex/fragment effect whose parameters the @ref Renderer sets before drawing.
 */
enum class Effect {
    None,
    Wind,
    Fire
};

enum class DrawMethod {
    /**
     * @brief One draw call per transform.
     */
    Default,
    /**
     * @brief Like @ref DrawMethod::Default, with alpha blending enabled.
     */
    Blended,
    /**
     * @brief One draw call per mesh for all visible transforms. Requires a shader that reads the instance matrix from attribute 5.
     */
    Instanced
};

/**
 * @class DrawBatch
 * @brief Static placements of a model. Model matrices and world-space bounds are computed once, when the batch is created
 * with @ref Renderer::create_batch, and instanced batches own their GPU instance buffer.
 */
class DrawBatch {
    friend class Renderer;

public:
    [[nodiscard]] const resources::Model *model() const {
        return m_model;
    }

    [[nodiscard]] DrawMethod method() const {
        return m_method;
    }

    [[nodiscard]] std::span<const glm::mat4> transforms() const {
        return m_transforms;
    }

    [[nodiscard]] std::span<const util::AABB> bounds() const {
        return m_bounds;
    }

private:
    DrawBatch(const resources::Model *model, std::span<const util::TransformData> transforms, DrawMethod method);

    void destroy() const;

    const resources::Model *m_model;
    DrawMethod m_method;
    std::vector<glm::mat4> m_transforms;
    std::vector<util::AABB> m_bounds;
    std::optional<resources::InstanceBuffer> m_instances;
};

}
