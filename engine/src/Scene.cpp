#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/Scene.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Errors.hpp>
#include <engine/util/Utils.hpp>
#include <format>
#include <json.hpp>
#include <optional>
#include <stdexcept>

namespace engine::graphics {

namespace {
using json = nlohmann::json;

std::optional<DrawMethod> parse_draw_method(const std::string_view name) {
    if (name == "default") return DrawMethod::Default;
    if (name == "blended") return DrawMethod::Blended;
    if (name == "instanced") return DrawMethod::Instanced;
    return std::nullopt;
}

std::optional<Effect> parse_effect(const std::string_view name) {
    if (name == "none") return Effect::None;
    if (name == "wind") return Effect::Wind;
    if (name == "fire") return Effect::Fire;
    return std::nullopt;
}

/**
 * @brief Reads `key` of `object` as a vec3 given as [x, y, z] or as a single number for all three components.
 */
glm::vec3 read_vec3(const json &object, const char *key, const glm::vec3 &fallback) {
    if (!object.contains(key)) {
        return fallback;
    }
    const json &value = object.at(key);
    if (value.is_number()) {
        return glm::vec3(value.get<float>());
    }
    if (!value.is_array() || value.size() != 3) {
        throw std::invalid_argument(std::format("\"{}\" must be a number or an array of 3 numbers", key));
    }
    return {value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
}

util::TransformData read_transform(const json &transform) {
    return {
            .translation = read_vec3(transform, "position", glm::vec3(0.0f)),
            .rotation = read_vec3(transform, "rotation", glm::vec3(0.0f)),
            .scale = read_vec3(transform, "scale", glm::vec3(1.0f)),
    };
}
}

Scene Scene::load(const std::filesystem::path &path) {
    if (!exists(path)) {
        throw util::EngineError(util::EngineError::Type::FileNotFound, std::format("Scene file {} not found.", path.string()));
    }
    json document;
    try {
        document = json::parse(util::read_text_file(path));
    } catch (const json::exception &error) {
        throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                std::format("Scene file {} is not valid JSON: {}", path.string(), error.what()));
    }

    const auto resources = core::Controller::get<resources::ResourcesController>();
    const auto renderer  = core::Controller::get<Renderer>();
    Scene scene;
    std::vector<glm::vec3> ground_triangles;
    const json &entities = document.value("entities", json::array());
    for (std::size_t index = 0; index < entities.size(); ++index) {
        const json &entity = entities[index];
        auto invalid = [&](const std::string_view problem) {
            return util::EngineError(util::EngineError::Type::ConfigurationError,
                                     std::format("Scene file {}, entity {}: {}", path.string(), index, problem));
        };
        try {
            const auto method = parse_draw_method(entity.value("method", "default"));
            if (!method) {
                throw invalid("unknown \"method\", expected \"default\", \"blended\" or \"instanced\".");
            }
            const auto effect = parse_effect(entity.value("effect", "none"));
            if (!effect) {
                throw invalid("unknown \"effect\", expected \"none\", \"wind\" or \"fire\".");
            }
            std::vector<util::TransformData> transforms;
            for (const json &transform: entity.at("transforms")) {
                transforms.push_back(read_transform(transform));
            }
            const auto model_name = entity.at("model").get<std::string>();
            const DrawBatch *batch = renderer->create_batch(resources->model(model_name), transforms, *method);
            if (entity.value("collider", false)) {
                const auto model_triangles = resources->collision_triangles(model_name);
                for (const glm::mat4 &transform: batch->transforms()) {
                    for (const glm::vec3 &corner: model_triangles) {
                        ground_triangles.emplace_back(transform * glm::vec4(corner, 1.0f));
                    }
                }
            }
            scene.m_entities.push_back({
                    .model_name = model_name,
                    .batch = batch,
                    .shader = resources->shader(entity.at("shader").get<std::string>()),
                    .effect = *effect,
                    .casts_shadow = entity.value("casts_shadow", false),
                    .group = entity.value("group", ""),
            });
        } catch (const json::exception &error) {
            throw invalid(error.what());
        } catch (const std::invalid_argument &error) {
            throw invalid(error.what());
        }
    }
    scene.m_ground = physics::HeightField(ground_triangles, GROUND_CELL_SIZE);
    return scene;
}

void Scene::draw(Renderer &renderer) const {
    for (const auto &entity: m_entities) {
        if (entity.visible) {
            renderer.draw(entity.batch, entity.shader, entity.effect);
        }
    }
}

void Scene::draw_shadow_casters(Renderer &renderer) const {
    for (const auto &entity: m_entities) {
        if (entity.visible && entity.casts_shadow) {
            renderer.draw_depth(entity.batch);
        }
    }
}

void Scene::set_group_visible(const std::string_view group, const bool visible) {
    for (auto &entity: m_entities) {
        if (entity.group == group) {
            entity.visible = visible;
        }
    }
}

}
