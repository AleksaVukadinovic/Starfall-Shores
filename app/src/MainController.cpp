#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/FPSCameraController.hpp>
#include <engine/graphics/FlockController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/LightingController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/graphics/Renderer.hpp>
#include <engine/graphics/ShadowController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/sound/SoundController.hpp>
#include <engine/util/Transform.hpp>
#include <engine/util/Utils.hpp>

namespace app {
    using vec3 = glm::vec3;

    void MainController::initialize() {
        engine::graphics::OpenGL::enable_depth_testing();
        const auto platform = get<engine::platform::PlatformController>();
        platform->set_enable_cursor(false);
        m_graphics = get<engine::graphics::GraphicsController>();
        m_bloom = get<engine::graphics::PostProcessingController>();
        m_fog               = get<FogController>();
        m_water             = get<engine::graphics::WaterController>();
        m_shadow            = get<engine::graphics::ShadowController>();
        m_lighting          = get<engine::graphics::LightingController>();
        m_renderer          = get<engine::graphics::Renderer>();
        m_graphics->perspective_params().Far = m_fog->far_plane();
        m_resources         = get<engine::resources::ResourcesController>();
        m_time = get<engine::platform::TimeController>();
        m_camera            = m_graphics->camera();
        m_is_day           = true;
        m_camera->Position = vec3(5, 27, 17);
        m_camera->rotate_camera(0, 0);
        apply_day_night_lighting();
        create_scene();
        // Skyboxes load on first use; load the two shown by default now, so switching to night doesn't stall a frame.
        m_resources->skybox(m_active_daytime_skybox);
        m_resources->skybox(m_active_nighttime_skybox);
        m_renderer->set_depth_scene([this] { render_depth_scene(); });
        get<engine::graphics::FPSCameraController>()->ground_height = [this](const glm::vec2 xz) { return solid_ground(xz); };
        get<engine::graphics::FlockController>()->ground_height = [this](const glm::vec2 xz) { return solid_ground(xz); };
        m_background_sound = get<engine::sound::SoundController>()->sound("ocean_wind.wav");
    }

    bool MainController::loop() {
        if (get<GUIController>()->m_wants_quit)
            return false;
        return true;
    }

    using engine::util::model_matrix;

    void MainController::create_scene() {
        m_scene = engine::graphics::Scene::load(SCENE_PATH);
        m_scene.set_group_visible(NIGHT_GROUP, !m_is_day);
        m_scene.set_group_visible(DAY_GROUP, m_is_day);
        m_was_underwater = m_water->camera_underwater();
        m_scene.set_group_visible(UNDERWATER_GROUP, m_was_underwater);
        m_placement_shader = m_resources->shader("basic");
    }

    void MainController::draw() {
        m_scene.draw(*m_renderer);
        draw_placed_models();
        draw_skybox();
    }

    void MainController::draw_skybox() const {
        const auto shader = m_resources->shader("skybox");
        const engine::resources::Skybox *skybox_cube = m_resources->skybox(m_is_day ? m_active_daytime_skybox : m_active_nighttime_skybox);
        m_graphics->draw_skybox(shader, skybox_cube);
        m_water->reflected_skybox = skybox_cube;
    }

    void MainController::update() {
        update_underwater();
        update_day_night_transition();
        update_toggles();
        update_sound();
    }

    void MainController::update_day_night_transition() {
        using namespace engine::platform;

        // The clock decides whether it's day or night; N jumps the clock to the next nightfall or daybreak.
        if (get<PlatformController>()->key(KEY_N).state() == Key::State::JustPressed && !m_day_change_requested) {
            m_time->set_hours(m_is_day ? TimeController::NIGHT_START : TimeController::NIGHT_END);
        }
        if (m_is_day == m_time->is_night() && !m_day_change_requested) {
            m_day_change_requested = true;
            m_day_change_timer = PlatformController::get_time();
        }

        if (m_day_change_requested) {
            const double elapsed_time = PlatformController::get_time() - m_day_change_timer;
            if (const auto transition_progress = static_cast<float>(elapsed_time / DAY_CHANGE_DELAY); transition_progress >= 1.0f) {
                m_is_day = !m_is_day;
                m_current_exposure = m_is_day ? DAY_EXPOSURE : NIGHT_EXPOSURE;
                m_fog->set_day(m_is_day);
                m_scene.set_group_visible(NIGHT_GROUP, !m_is_day);
                m_scene.set_group_visible(DAY_GROUP, m_is_day);
                m_day_change_requested = false;
                apply_day_night_lighting();
                if (!m_is_day)
                    m_renderer->fire.start_time = PlatformController::get_time();
            } else {
                const float start_exposure = m_is_day ? DAY_EXPOSURE : NIGHT_EXPOSURE;
                const float target_exposure = m_is_day ? NIGHT_EXPOSURE : DAY_EXPOSURE;
                m_current_exposure = start_exposure + (target_exposure - start_exposure) * transition_progress;
                m_fog->transition(transition_progress, m_is_day);
            }
            m_bloom->exposure = m_current_exposure;
        }
    }

    void MainController::update_toggles() {
        using namespace engine::platform;
        const auto platform = get<PlatformController>();
        if (platform->key(KEY_F).state() == Key::State::JustPressed)
            get<FogController>()->fog_enabled = !get<FogController>()->fog_enabled;
        if (platform->key(KEY_V).state() == Key::State::JustPressed)
            m_renderer->wind.enabled = !m_renderer->wind.enabled;
        if (platform->key(KEY_R).state() == Key::State::JustPressed)
            get<RainController>()->rain_enabled = !get<RainController>()->rain_enabled;
        if (platform->key(KEY_M).state() == Key::State::JustPressed)
            m_sound_enabled = !m_sound_enabled;
    }

    void MainController::update_sound() const {
        if (!m_background_sound) return;
        if (m_sound_enabled) {
            m_background_sound->set_volume(m_sound_volume / 100.0f);
            if (!m_background_sound->is_playing()) {
                m_background_sound->play_looping();
            }
        } else {
            if (m_background_sound->is_playing()) {
                m_background_sound->stop();
            }
        }
    }

    void MainController::apply_day_night_lighting() const {
        auto &[position, ambient, diffuse, specular, color, shininess] = m_lighting->light;
        position  = m_is_day ? LIGHT_POS_DAY : LIGHT_POS_NIGHT;
        ambient   = m_is_day ? AMBIENT_LIGHT_DAY : AMBIENT_LIGHT_NIGHT;
        diffuse   = m_is_day ? DIFFUSE_LIGHT_DAY : DIFFUSE_LIGHT_NIGHT;
        specular  = m_is_day ? SPECULAR_LIGHT_DAY : SPECULAR_LIGHT_NIGHT;
        color     = m_is_day ? LIGHT_COLOR_DAY : LIGHT_COLOR_NIGHT;
        shininess = m_is_day ? SHININESS_DAY : SHININESS_NIGHT;
        m_shadow->light_position = position;

        m_lighting->clear_point_lights();
        if (!m_is_day) {
            engine::graphics::PointLightSource fire_light;
            fire_light.position = glm::vec3(12.0f, 22.0f, 6.5f);
            fire_light.diffuse  = glm::vec3(1.0f, 0.6f, 0.2f);
            fire_light.specular = glm::vec3(1.0f, 0.4f, 0.1f);
            fire_light.constant  = 1.0f;
            fire_light.linear    = 0.045f;
            fire_light.quadratic = 0.0075f;
            m_lighting->add_point_light(fire_light);
        }
    }

    void MainController::draw_placed_models() const {
        for (const auto &[model_name, model, translation, rotation, scale] : placed_models) {
            m_renderer->draw(model, m_placement_shader, model_matrix(translation, rotation, vec3(scale)));
        }
        if (model_placement_active) {
            const auto *model = m_resources->model(TEST_MODEL_NAMES[selected_model_index]);
            m_renderer->draw(model, m_placement_shader, model_matrix(test_translation, test_rotation, vec3(test_scale)));
        }
    }

    void MainController::place_test_model() {
        const auto *model_name = TEST_MODEL_NAMES[selected_model_index];
        placed_models.push_back({model_name, m_resources->model(model_name), test_translation, test_rotation, test_scale});
    }

    void MainController::clear_placed_models() {
        placed_models.clear();
    }

    void MainController::terminate() {
        // Placed models are logged as scene file transforms, ready to paste into the entity of their model.
        for (const auto &[model_name, model, translation, rotation, scale] : placed_models) {
            spdlog::info(std::format("{}: {{\"position\": [{:.3g}, {:.3g}, {:.3g}], \"rotation\": [{:.3g}, {:.3g}, {:.3g}], \"scale\": {:.3g}}}",
                model_name,
                translation.x, translation.y, translation.z,
                rotation.x, rotation.y, rotation.z,
                scale));
        }
    }

    void MainController::set_skybox(const std::string &new_skybox, const bool is_daytime_skybox) {
        if (is_daytime_skybox) {
            m_active_daytime_skybox = new_skybox;
        } else {
            m_active_nighttime_skybox = new_skybox;
        }
    }

    std::optional<float> MainController::solid_ground(const glm::vec2 xz) const {
        const auto terrain = m_scene.ground().height_at(xz);
        return std::max(terrain.value_or(engine::graphics::WaterController::SEA_FLOOR_HEIGHT), engine::graphics::WaterController::SEA_FLOOR_HEIGHT);
    }

    void MainController::update_underwater() {
        const bool underwater = m_water->camera_underwater();
        if (underwater != m_was_underwater) {
            m_scene.set_group_visible(UNDERWATER_GROUP, underwater);
            m_was_underwater = underwater;
        }
        m_bloom->light_shafts = underwater ? m_water->light_shafts : 0.0f;
        get<engine::graphics::FPSCameraController>()->water_level = m_water->level;

        // Sounds fade to muffled when diving under, and back when surfacing.
        const float dt = get<engine::platform::PlatformController>()->dt();
        m_muffling = engine::util::approach(m_muffling, underwater ? 1.0f : 0.0f, dt / MUFFLING_FADE_SECONDS);
        get<engine::sound::SoundController>()->set_muffling(m_muffling);
    }

    void MainController::render_depth_scene() const {
        if (m_is_day)
            m_scene.draw_shadow_casters(*m_renderer);
    }
}
