#include <Constants.hpp>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Controller.hpp>
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

namespace app {
    using vec3 = glm::vec3;
    using engine::util::TransformData;

    void MainController::initialize() {
        engine::graphics::OpenGL::enable_depth_testing();
        const auto platform = get<engine::platform::PlatformController>();
        platform->set_enable_cursor(false);
        m_graphics = get<engine::graphics::GraphicsController>();
        m_bloom = get<engine::graphics::PostProcessingController>();
        m_fog               = get<FogController>();
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
        m_renderer->set_depth_scene([this] { render_depth_scene(); });
        m_background_sound = get<engine::sound::SoundController>()->sound("ocean_wind.wav");
    }

    bool MainController::loop() {
        if (get<GUIController>()->m_wants_quit)
            return false;
        return true;
    }

    using engine::util::model_matrix;

    static constexpr std::array<TransformData, 1> WATER = {{{{0, 0, 7}, {-90, 0, 0}, {30, 30, 1}}}};
    static constexpr std::array<TransformData, 2> TERRAIN = {{
        {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}},
        {{-8.15f, 2.3f, -89.5f}, {0, 0, 0}, {0.72f, 0.72f, 0.72f}},
    }};
    static constexpr std::array<TransformData, 1> VIKING_TENT = {{{{16, 17, -14}, {0, -20, 0}, {0.037f, 0.037f, 0.037f}}}};
    static constexpr std::array<TransformData, 1> STYLIZED_TENT = {{{{0, 20, -33}, {0, -128, 0}, {0.06f, 0.06f, 0.06f}}}};
    static constexpr std::array<TransformData, 1> GRAVE = {{{{29, 71, 12}, {-90, 0, -48}, {1.35f, 1.35f, 1.35f}}}};
    static constexpr std::array<TransformData, 1> CAMPFIRE = {{{{12.0f, 17.3f, 6.0f}}}};
    static constexpr std::array<TransformData, 1> LIGHTHOUSE = {{{{-102.0f, 15.0f, -107.0f}, {0, 0, 0}, {5, 5, 5}}}};
    static constexpr std::array<TransformData, 1> FIRE = {{{{12, 20.5f, 6.5f}, {0, 0, 0}, {3.1f, 3.1f, 3.1f}}}};

    static constexpr std::array<TransformData, 3> LOGS = {{
        #include <coordinates/logs.include>
    }};

    static constexpr std::array<TransformData, 5> SHROOM_POSITIONS = {{
        #include <coordinates/shrooms.include>
    }};

    constexpr std::array<TransformData, 68> white_translations = {{
        #include <coordinates/white_flowers.include>
    }};
    constexpr std::array<TransformData, 31> red_translations = {{
        #include <coordinates/red_flowers.include>
    }};

    constexpr std::array<TransformData, 35> grass_positions = {{
        #include <coordinates/grass.include>
    }};

    constexpr std::array<TransformData, 22> yellow_trees = {{
        #include <coordinates/yellow_trees.include>
    }};
    constexpr std::array<TransformData, 16> green_trees = {{
        #include <coordinates/green_trees.include>
    }};
    constexpr std::array<TransformData, 5> tall_trees = {{
        #include <coordinates/tall_trees.include>
    }};
    constexpr std::array<TransformData, 75> pine_trees = {{
        #include <coordinates/pine_trees.include>
    }};
    constexpr std::array<TransformData, 1> oak_trees = {{
        #include <coordinates/oak_trees.include>
    }};
    constexpr std::array<TransformData, 2> old_trees = {{
        #include <coordinates/old_trees.include>
    }};

    constexpr std::array<TransformData, 5> bush1_positions = {{
        #include <coordinates/bush1.include>
    }};
    constexpr std::array<TransformData, 3> bush2_positions = {{
        #include <coordinates/bush2.include>
    }};
    constexpr std::array<TransformData, 6> laurel_positions = {{
        #include <coordinates/laurel_bushes.include>
    }};

    constexpr std::array<TransformData, 19> path_segments = {{
        #include <coordinates/path_segments.include>
    }};

    using engine::graphics::DrawMethod;
    using engine::graphics::Effect;

    struct SceneObjectDescription {
        std::string_view model;
        std::string_view shader;
        std::span<const TransformData> transforms;
        DrawMethod method = DrawMethod::Default;
        Effect effect     = Effect::None;
        bool casts_shadow = false;
    };

    /**
     * @brief Everything the scene draws, in drawing order. Resolved into @ref SceneObject instances once, in @ref MainController::create_scene.
     */
    static constexpr std::array SCENE = {
        SceneObjectDescription{.model = "water", .shader = "water_shader", .transforms = WATER, .method = DrawMethod::Blended, .effect = Effect::Water},
        SceneObjectDescription{.model = "terrain", .shader = "basic", .transforms = TERRAIN, .casts_shadow = true},
        SceneObjectDescription{.model = "viking_tent", .shader = "basic", .transforms = VIKING_TENT, .casts_shadow = true},
        SceneObjectDescription{.model = "stylized_tent", .shader = "basic", .transforms = STYLIZED_TENT, .casts_shadow = true},
        SceneObjectDescription{.model = "grave", .shader = "basic", .transforms = GRAVE, .casts_shadow = true},
        SceneObjectDescription{.model = "campfire", .shader = "basic", .transforms = CAMPFIRE, .casts_shadow = true},
        SceneObjectDescription{.model = "lighthouse", .shader = "basic", .transforms = LIGHTHOUSE},
        SceneObjectDescription{.model = "log_seat", .shader = "basic", .transforms = LOGS, .casts_shadow = true},
        SceneObjectDescription{.model = "bush1", .shader = "basic", .transforms = bush1_positions, .method = DrawMethod::Blended},
        SceneObjectDescription{.model = "bush2", .shader = "basic", .transforms = bush2_positions, .method = DrawMethod::Blended},
        SceneObjectDescription{.model = "laurel_bush", .shader = "basic", .transforms = laurel_positions, .method = DrawMethod::Blended},
        SceneObjectDescription{.model = "yellow_tree", .shader = "flower_shader", .transforms = yellow_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "green_tree", .shader = "flower_shader", .transforms = green_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "beech_tree", .shader = "flower_shader", .transforms = tall_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "pine_tree", .shader = "flower_shader", .transforms = pine_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "oak_tree", .shader = "flower_shader", .transforms = oak_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "old_tree", .shader = "flower_shader", .transforms = old_trees, .method = DrawMethod::Instanced, .casts_shadow = true},
        SceneObjectDescription{.model = "white_flowers", .shader = "flower_shader", .transforms = white_translations, .method = DrawMethod::Instanced, .effect = Effect::Wind},
        SceneObjectDescription{.model = "red_flowers", .shader = "flower_shader", .transforms = red_translations, .method = DrawMethod::Instanced, .effect = Effect::Wind},
        SceneObjectDescription{.model = "path", .shader = "flower_shader", .transforms = path_segments, .method = DrawMethod::Instanced},
        SceneObjectDescription{.model = "shrooms", .shader = "basic", .transforms = SHROOM_POSITIONS, .casts_shadow = true},
        SceneObjectDescription{.model = "grass", .shader = "grass_shader", .transforms = grass_positions, .method = DrawMethod::Instanced, .effect = Effect::Wind, .casts_shadow = true},
    };

    void MainController::create_scene() {
        auto create_object = [this](const SceneObjectDescription &description) {
            const auto *model = m_resources->model(std::string(description.model));
            return SceneObject{
                .batch        = m_renderer->create_batch(model, description.transforms, description.method),
                .shader       = m_resources->shader(std::string(description.shader)),
                .effect       = description.effect,
                .casts_shadow = description.casts_shadow,
            };
        };
        m_scene_objects.reserve(SCENE.size());
        for (const auto &description: SCENE) {
            m_scene_objects.push_back(create_object(description));
        }
        m_fire = create_object({.model = "fire", .shader = "fire_shader", .transforms = FIRE, .method = DrawMethod::Blended, .effect = Effect::Fire});
        m_placement_shader = m_resources->shader("basic");
    }

    void MainController::draw() {
        for (const auto &object: m_scene_objects) {
            m_renderer->draw(object.batch, object.shader, object.effect);
        }
        draw_placed_models();
        if (!m_is_day)
            m_renderer->draw(m_fire.batch, m_fire.shader, m_fire.effect);
        draw_skybox();
    }

    void MainController::draw_skybox() const {
        const auto shader = m_resources->shader("skybox");
        const engine::resources::Skybox *skybox_cube = m_resources->skybox(m_is_day ? m_active_daytime_skybox : m_active_nighttime_skybox);
        m_graphics->draw_skybox(shader, skybox_cube);
    }

    void MainController::update() {
        m_bloom->underwater = m_camera->Position.y < 7.0f;
        update_day_night_transition();
        update_toggles();
        update_sound();
    }

    void MainController::update_day_night_transition() {
        using namespace engine::platform;

        if (const auto platform = get<PlatformController>(); platform->key(KEY_N).state() == Key::State::JustPressed ||
            (m_time->hours() >= 23 && m_is_day && !m_day_change_requested) ||
            (m_time->hours() >= 7 && !m_is_day && !m_day_change_requested)) {
            if (!m_day_change_requested) {
                m_day_change_requested = true;
                m_day_change_timer = PlatformController::get_time();
            }
        }

        if (m_day_change_requested) {
            const double elapsed_time = PlatformController::get_time() - m_day_change_timer;
            if (const auto transition_progress = static_cast<float>(elapsed_time / DAY_CHANGE_DELAY); transition_progress >= 1.0f) {
                m_is_day = !m_is_day;
                m_current_exposure = m_is_day ? DAY_EXPOSURE : NIGHT_EXPOSURE;
                m_fog->set_day(m_is_day);
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
        m_renderer->water.color = m_is_day ? glm::vec3(0.0f, 0.4f, 0.6f) : glm::vec3(0.0f, 0.0f, 0.08f);

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
        for (const auto &[model_name, model, translation, rotation, scale] : placed_models) {
            spdlog::info(std::format("\n// {}\n{{{{{:.3g}f, {:.3g}f, {:.3g}f}}, {{{:.3g}f, {:.3g}f, {:.3g}f}}, {{{:.3g}f, {:.3g}f, {:.3g}f}}}}",
                model_name,
                translation.x, translation.y, translation.z,
                rotation.x, rotation.y, rotation.z,
                scale, scale, scale));
        }
    }

    void MainController::set_skybox(const std::string &new_skybox, const bool is_daytime_skybox) {
        if (is_daytime_skybox) {
            m_active_daytime_skybox = new_skybox;
        } else {
            m_active_nighttime_skybox = new_skybox;
        }
    }

    void MainController::render_depth_scene() const {
        if (!m_is_day) return;
        for (const auto &object: m_scene_objects) {
            if (object.casts_shadow)
                m_renderer->draw_depth(object.batch);
        }
    }
}
