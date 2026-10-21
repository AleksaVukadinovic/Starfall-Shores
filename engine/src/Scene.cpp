#include <engine/graphics/FlockController.hpp>
#include <engine/graphics/ParticleController.hpp>
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

std::optional<ParticleBlend> parse_particle_blend(const std::string_view name) {
    if (name == "additive") return ParticleBlend::Additive;
    if (name == "alpha") return ParticleBlend::Alpha;
    return std::nullopt;
}

/**
 * @brief Reads `key` of `object` as a vector given as an array of N numbers, or as a single number for all components.
 */
template<glm::length_t N>
glm::vec<N, float> read_vector(const json &object, const char *key, const glm::vec<N, float> &fallback) {
    if (!object.contains(key)) {
        return fallback;
    }
    const json &value = object.at(key);
    if (value.is_number()) {
        return glm::vec<N, float>(value.get<float>());
    }
    if (!value.is_array() || value.size() != N) {
        throw std::invalid_argument(std::format("\"{}\" must be a number or an array of {} numbers", key, N));
    }
    glm::vec<N, float> result;
    for (glm::length_t i = 0; i < N; ++i) {
        result[i] = value[i].get<float>();
    }
    return result;
}

util::TransformData read_transform(const json &transform) {
    return {
            .translation = read_vector<3>(transform, "position", glm::vec3(0.0f)),
            .rotation = read_vector<3>(transform, "rotation", glm::vec3(0.0f)),
            .scale = read_vector<3>(transform, "scale", glm::vec3(1.0f)),
    };
}

FlockSettings read_flock(const json &object) {
    FlockSettings flock;
    flock.name                = object.value("name", flock.name);
    flock.count               = object.at("count").get<uint32_t>();
    flock.center              = read_vector<3>(object, "center", flock.center);
    flock.extent              = read_vector<3>(object, "extent", flock.extent);
    flock.length              = object.value("length", flock.length);
    flock.model_rotation      = read_vector<3>(object, "model_rotation", flock.model_rotation);
    flock.min_speed           = object.value("min_speed", flock.min_speed);
    flock.max_speed           = object.value("max_speed", flock.max_speed);
    flock.neighbour_radius    = object.value("neighbour_radius", flock.neighbour_radius);
    flock.separation_distance = object.value("separation_distance", flock.separation_distance);
    flock.ground_clearance    = object.value("ground_clearance", flock.ground_clearance);
    return flock;
}

ParticleEmitter read_particle_emitter(const json &object) {
    ParticleEmitter emitter;
    const auto blend = parse_particle_blend(object.value("blend", "additive"));
    if (!blend) {
        throw std::invalid_argument("unknown \"blend\", expected \"additive\" or \"alpha\".");
    }
    const glm::vec2 size = read_vector<2>(object, "size", glm::vec2(emitter.size_start, emitter.size_end));
    emitter.name                 = object.value("name", emitter.name);
    emitter.group                = object.value("group", emitter.group);
    emitter.position             = read_vector<3>(object, "position", emitter.position);
    emitter.extent               = read_vector<3>(object, "extent", emitter.extent);
    emitter.follow_camera        = object.value("follow_camera", emitter.follow_camera);
    emitter.count                = object.at("count").get<uint32_t>();
    emitter.lifetime             = object.value("lifetime", emitter.lifetime);
    emitter.lifetime_variation   = object.value("lifetime_variation", emitter.lifetime_variation);
    emitter.velocity             = read_vector<3>(object, "velocity", emitter.velocity);
    emitter.velocity_variation   = read_vector<3>(object, "velocity_variation", emitter.velocity_variation);
    emitter.acceleration         = read_vector<3>(object, "acceleration", emitter.acceleration);
    emitter.turbulence           = object.value("turbulence", emitter.turbulence);
    emitter.turbulence_frequency = object.value("turbulence_frequency", emitter.turbulence_frequency);
    emitter.wind_influence       = object.value("wind_influence", emitter.wind_influence);
    emitter.size_start           = size.x;
    emitter.size_end             = size.y;
    emitter.color_start          = read_vector<4>(object, "color_start", emitter.color_start);
    emitter.color_end            = read_vector<4>(object, "color_end", emitter.color_end);
    emitter.blend                = *blend;
    emitter.lit                  = object.value("lit", emitter.lit);
    emitter.blink_frequency      = object.value("blink_frequency", emitter.blink_frequency);
    emitter.fade_seconds         = object.value("fade_seconds", emitter.fade_seconds);
    return emitter;
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

    const json &emitters = document.value("particles", json::array());
    for (std::size_t index = 0; index < emitters.size(); ++index) {
        try {
            core::Controller::get<ParticleController>()->add_emitter(read_particle_emitter(emitters[index]));
        } catch (const std::exception &error) {
            throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                    std::format("Scene file {}, particle emitter {}: {}", path.string(), index, error.what()));
        }
    }
    scene.m_has_particles = !emitters.empty();

    const json &flocks = document.value("flocks", json::array());
    for (std::size_t index = 0; index < flocks.size(); ++index) {
        try {
            const json &flock = flocks[index];
            core::Controller::get<FlockController>()->add_flock(read_flock(flock), resources->model(flock.at("model").get<std::string>()),
                                                                resources->shader(flock.at("shader").get<std::string>()));
        } catch (const std::exception &error) {
            throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                    std::format("Scene file {}, flock {}: {}", path.string(), index, error.what()));
        }
    }
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
    if (m_has_particles) {
        core::Controller::get<ParticleController>()->set_group_visible(group, visible);
    }
}

}
