#include <engine/graphics/DrawBatch.hpp>
#include <engine/resources/Model.hpp>

namespace engine::graphics {

DrawBatch::DrawBatch(const resources::Model *model, const std::span<const util::TransformData> transforms, const DrawMethod method)
    : m_model(model)
    , m_method(method) {
    m_transforms.reserve(transforms.size());
    m_bounds.reserve(transforms.size());
    for (const auto &transform: transforms) {
        const glm::mat4 &matrix = m_transforms.emplace_back(util::model_matrix(transform));
        m_bounds.push_back(model->bounds().transformed(matrix));
    }
    if (method == DrawMethod::Instanced) {
        m_instances.emplace(*model);
    }
}

void DrawBatch::destroy() const {
    if (m_instances) {
        m_instances->destroy();
    }
}

}
