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
#include <engine/util/Transform.hpp>

namespace app {
    using vec3 = glm::vec3;
    using mat4 = glm::mat4;
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
        m_camera            = m_graphics->camera();
        m_is_day           = true;
        m_camera->Position = vec3(5, 27, 17);
        m_camera->rotate_camera(0, 0);
        apply_day_night_lighting();
        m_renderer->set_depth_scene([this] { render_depth_scene(); });
    }

    bool MainController::loop() {
        if (const auto platform = get<engine::platform::PlatformController>(); platform->key(
                engine::platform::KeyId::KEY_ESCAPE).is_down() && !get<GUIController>()->is_enabled())
            return false;
        return true;
    }

    using engine::util::model_matrix;
    using engine::util::build_instance_matrices;
    using engine::util::build_instance_matrices_async;

    constexpr std::array<TransformData, 3> LOGS = {{
        #include <coordinates/logs.include>
    }};

    constexpr std::array<TransformData, 5> SHROOM_POSITIONS = {{
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

    using Effect = engine::graphics::Effect;
    using DrawMethod = engine::graphics::DrawMethod;

    void MainController::draw() {
        m_renderer->draw_blended("water", "water_shader", model_matrix(vec3(0, 0, 7), vec3(30, 30, 1), X_AXIS, -90.0f), Effect::Water);
        m_renderer->draw("terrain", "basic", mat4(1.0f));
        m_renderer->draw("terrain", "basic", model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        m_renderer->draw("viking_tent", "basic", model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f));
        m_renderer->draw("stylized_tent", "basic", model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f));
        m_renderer->draw("grave", "basic", model_matrix(vec3(29, 71, 12), vec3(-90, 0, -48), vec3(1.35)));
        m_renderer->draw("campfire", "basic", model_matrix(vec3(12.0f, 17.3f, 6.0f)));
        m_renderer->draw_batch("log_seat", "basic", LOGS);
        m_renderer->draw_batch("bush1", "basic", bush1_positions, DrawMethod::Blended);
        m_renderer->draw_batch("bush2", "basic", bush2_positions, DrawMethod::Blended);
        m_renderer->draw_batch("laurel_bush", "basic", laurel_positions, DrawMethod::Blended);
        m_renderer->draw_batch_instanced("yellow_tree", "flower_shader", yellow_trees);
        m_renderer->draw_batch_instanced("green_tree", "flower_shader", green_trees);
        m_renderer->draw_batch_instanced("beech_tree", "flower_shader", tall_trees);
        m_renderer->draw_batch_instanced("pine_tree", "flower_shader", pine_trees);
        m_renderer->draw_batch_instanced("oak_tree", "flower_shader", oak_trees);
        m_renderer->draw_batch_instanced("old_tree", "flower_shader", old_trees);
        m_renderer->draw_batch_instanced("white_flowers", "flower_shader", white_translations, Effect::Wind);
        m_renderer->draw_batch_instanced("red_flowers", "flower_shader", red_translations, Effect::Wind);
        m_renderer->draw_batch_instanced("path", "flower_shader", path_segments);
        m_renderer->draw_batch("shrooms", "basic", SHROOM_POSITIONS);
        m_renderer->draw_batch_instanced("grass", "grass_shader", grass_positions, Effect::Wind);
        draw_test_model();
        if (!m_is_day)
            m_renderer->draw_blended("fire", "fire_shader", model_matrix(vec3(12, 20.5, 6.5), vec3(3.1), Y_AXIS, 0.0f), Effect::Fire);
        draw_skybox();
    }

    void MainController::draw_skybox() const {
        const auto shader = m_resources->shader("skybox");
        shader->use();
        m_fog->apply_to_shader(shader);
        const engine::resources::Skybox *skybox_cube = m_resources->skybox(m_is_day ? m_active_daytime_skybox : m_active_nighttime_skybox);
        m_graphics->draw_skybox(shader, skybox_cube);
    }

    void MainController::update() {
        m_bloom->underwater = m_camera->Position.y < 7.0f;
        update_day_night_transition();
        update_toggles();
    }

    void MainController::update_day_night_transition() {
        using namespace engine::platform;
        if (const auto platform = get<PlatformController>(); platform->key(KEY_N).state() == Key::State::JustPressed) {
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

    void MainController::update_toggles() const {
        using namespace engine::platform;
        const auto platform = get<PlatformController>();
        if (platform->key(KEY_F).state() == Key::State::JustPressed)
            get<FogController>()->fog_enabled = !get<FogController>()->fog_enabled;
        if (platform->key(KEY_V).state() == Key::State::JustPressed)
            m_renderer->wind.enabled = !m_renderer->wind.enabled;
        if (platform->key(KEY_R).state() == Key::State::JustPressed)
            get<RainController>()->rain_enabled = !get<RainController>()->rain_enabled;
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

    void MainController::draw_test_model() const {
        auto draw_with_transform = [&](const std::string &model_name, const vec3 &translation, const vec3 &rotation, const float scale) {
            m_renderer->draw(model_name, "basic", model_matrix(translation, rotation, vec3(scale)));
        };

        for (const auto &[model_name, translation, rotation, scale] : placed_models) {
            draw_with_transform(model_name, translation, rotation, scale);
        }

        const auto &current_name = TEST_MODEL_NAMES[selected_model_index];
        draw_with_transform(current_name, test_translation, test_rotation, test_scale);
    }

    void MainController::place_test_model() {
        placed_models.push_back({
            TEST_MODEL_NAMES[selected_model_index],
            test_translation,
            test_rotation,
            test_scale
        });
    }

    void MainController::clear_placed_models() {
        placed_models.clear();
    }

    void MainController::terminate() {
        for (const auto &[model_name, translation, rotation, scale] : placed_models) {
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
        m_renderer->setup_depth_shader();
        m_renderer->draw_depth("terrain", mat4(1.0f));
        m_renderer->draw_depth("terrain", model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        m_renderer->draw_depth("campfire", model_matrix(vec3(12.0f, 17.3f, 6.0f)));
        m_renderer->draw_depth("viking_tent", model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f));
        m_renderer->draw_depth("stylized_tent", model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f));
        m_renderer->draw_depth("grave", model_matrix(vec3(29, 71, 12), vec3(-90, 0, -48), vec3(1.35)));
        m_renderer->draw_depth_batch("shrooms", SHROOM_POSITIONS);
        m_renderer->draw_depth_batch("log_seat", LOGS);

        m_renderer->setup_depth_instanced_shader();
        m_renderer->draw_depth_batch_instanced("yellow_tree", yellow_trees);
        m_renderer->draw_depth_batch_instanced("green_tree", green_trees);
        m_renderer->draw_depth_batch_instanced("beech_tree", tall_trees);
        m_renderer->draw_depth_batch_instanced("pine_tree", pine_trees);
        m_renderer->draw_depth_batch_instanced("oak_tree", oak_trees);
        m_renderer->draw_depth_batch_instanced("old_tree", old_trees);
        m_renderer->draw_depth_batch_instanced("grass", grass_positions);
    }
}
