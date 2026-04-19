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
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/util/Transform.hpp>

namespace app {
    using vec3 = glm::vec3;
    using mat4 = glm::mat4;

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
        m_basic_shader      = m_resources->shader("basic");
        m_depth_shader      = m_resources->shader("depth_shader");
        m_depth_instanced_shader = m_resources->shader("depth_instanced_shader");
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

    constexpr std::array<std::pair<float, vec3>, 3> LOGS = {{
        #include <coordinates/logs.include>
    }};

    constexpr std::array SHROOM_POSITIONS = {
        #include <coordinates/shrooms.include>
    };

    void MainController::draw() {
        draw_water();
        m_renderer->draw("terrain", "basic", mat4(1.0f));
        m_renderer->draw("terrain", "basic", model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        draw_campfire();
        for (const auto &[angle, pos] : LOGS)
            m_renderer->draw("log_seat", "basic", model_matrix(pos, vec3(0.04f), Y_AXIS, angle));
        m_renderer->draw("viking_tent", "basic", model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f));
        m_renderer->draw("stylized_tent", "basic", model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f));
        draw_trees(m_resources->shader("flower_shader"));
        draw_bushes();
        draw_flowers();
        draw_path();
        for (const auto &shroom : SHROOM_POSITIONS)
            m_renderer->draw("shrooms", "basic", model_matrix(shroom, vec3(0.19f), X_AXIS, -90.0f));
        m_renderer->draw("grave", "basic", model_matrix(vec3(29, 71, 12), vec3(-90, 0, -48), vec3(1.35)));
        draw_grass();
        draw_test_model();
        if (!m_is_day)
            draw_fire();
        draw_skybox();
    }

    void MainController::draw_trees(const engine::resources::Shader *shader) const {
        constexpr std::array<TreeData, 22> yellow_trees = {{
            #include <coordinates/yellow_trees.include>
        }};
        constexpr std::array<TreeData, 16> green_trees = {{
            #include <coordinates/green_trees.include>
        }};
        constexpr std::array<TreeData, 5> tall_trees = {{
            #include <coordinates/tall_trees.include>
        }};
        constexpr std::array<TreeData, 75> pine_trees = {{
            #include <coordinates/pine_trees.include>
        }};
        constexpr std::array<TreeData, 1> oak_trees = {{
            #include <coordinates/oak_trees.include>
        }};
        constexpr std::array<OldTreeData, 2> old_trees = {{
            #include <coordinates/old_trees.include>
        }};

        auto tree_transform = [](const TreeData &t, const vec3 &axis, const float angle) {
            return model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), axis, angle);
        };

        auto yellow_f = build_instance_matrices_async(yellow_trees, [&](const TreeData &t) { return tree_transform(t, Y_AXIS, 0.0f); });
        auto green_f = build_instance_matrices_async(green_trees, [&](const TreeData &t) { return tree_transform(t, X_AXIS, -90.0f); });
        auto tall_f = build_instance_matrices_async(tall_trees, [&](const TreeData &t) { return tree_transform(t, Y_AXIS, 0.0f); });
        auto pine_f = build_instance_matrices_async(pine_trees, [&](const TreeData &t) { return tree_transform(t, X_AXIS, -90.0f); });
        auto oak_f = build_instance_matrices_async(oak_trees, [&](const TreeData &t) { return tree_transform(t, X_AXIS, 90.0f); });
        auto old_f = build_instance_matrices_async(old_trees, [](const OldTreeData &t) {
            return model_matrix(vec3(t.x, t.y, t.z), vec3(t.scale), vec3(t.rx, t.ry, t.rz), t.rotation_angle);
        });

        m_renderer->draw_instanced(m_resources->model("yellow_tree"), shader, yellow_f.get());
        m_renderer->draw_instanced(m_resources->model("green_tree"), shader, green_f.get());
        m_renderer->draw_instanced(m_resources->model("beech_tree"), shader, tall_f.get());
        m_renderer->draw_instanced(m_resources->model("pine_tree"), shader, pine_f.get());
        m_renderer->draw_instanced(m_resources->model("oak_tree"), shader, oak_f.get());
        m_renderer->draw_instanced(m_resources->model("old_tree"), shader, old_f.get());
    }

    void MainController::draw_campfire() const {
        m_lighting->apply_to_shader(m_basic_shader);
        m_basic_shader->set_vec3("light.diffuse", m_is_day ? vec3(0.5f) : vec3(5.0f));
        m_basic_shader->set_mat4("model", model_matrix(vec3(12.0f, 17.3f, 6.0f)));
        m_resources->model("campfire")->draw(m_basic_shader);
    }

    void MainController::draw_bushes() const {
        using BushData = std::array<float, 4>;
        auto draw_bush = [&](const std::string &model_name, const BushData &data, const float rotation_angle = 0.0f, const vec3 &rotation_axis = Y_AXIS) {
            m_renderer->draw_blended(model_name, "basic", model_matrix(vec3(data[0], data[1], data[2]), vec3(data[3]), rotation_axis, rotation_angle));
        };

        constexpr std::array<BushData, 5> bush1_positions = {{
            #include <coordinates/bush1.include>
        }};
        constexpr std::array<BushData, 3> bush2_positions = {{
            #include <coordinates/bush2.include>
        }};
        constexpr std::array<BushData, 6> laurel_positions = {{
            #include <coordinates/laurel_bushes.include>
        }};

        for (const auto &b : bush1_positions) draw_bush("bush1", b, -90.0f, X_AXIS);
        for (const auto &b : bush2_positions) draw_bush("bush2", b);
        for (const auto &b : laurel_positions) draw_bush("laurel_bush", b);
    }

    void MainController::draw_flowers() const {
        const auto *flower_shader = m_resources->shader("flower_shader");
        m_lighting->apply_to_shader(flower_shader);
        flower_shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time()));
        flower_shader->set_bool("windEnabled", wind_enabled);
        flower_shader->set_float("windIntensity", wind_intensity);

        constexpr std::array white_translations = {
            #include <coordinates/white_flowers.include>
        };
        constexpr std::array red_translations = {
            #include <coordinates/red_flowers.include>
        };
        auto white_matrices_f = build_instance_matrices_async(white_translations, [](const vec3 &t) {
            return model_matrix(t, vec3(0.12f), X_AXIS, 90.0f);
        });
        auto red_matrices_f = build_instance_matrices_async(red_translations, [](const vec3 &t) {
            return model_matrix(t, vec3(0.04f), X_AXIS, -90.0f);
        });

        m_resources->model("white_flowers")->draw_instanced(flower_shader, white_matrices_f.get());
        m_resources->model("red_flowers")->draw_instanced(flower_shader, red_matrices_f.get());
    }

    void MainController::draw_path() const {
        struct PathData { float rx, ry, rz, tx, ty, tz, scale; };
        constexpr std::array<PathData, 19> path_segments = {{
            #include <coordinates/path_segments.include>
        }};

        std::vector<mat4> matrices;
        matrices.reserve(path_segments.size());
        for (const auto &[rx, ry, rz, tx, ty, tz, s] : path_segments) {
            matrices.push_back(model_matrix(vec3(tx, ty, tz), vec3(rx, ry, rz), vec3(s)));
        }
        m_renderer->draw_instanced("path", "flower_shader", matrices);
    }

    void MainController::draw_water() const {
        const engine::resources::Model *water = m_resources->model("water");
        const engine::resources::Shader *shader = m_resources->shader("water_shader");

        m_lighting->apply_to_shader(shader);
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time()));
        shader->set_vec3("waterColor", m_is_day? WATER_COLOR_DAY: WATER_COLOR_NIGHT);
        shader->set_vec3("lightPos", m_lighting->light.position);

        const auto model = model_matrix(vec3(0, 0, 7), vec3(30, 30, 1), X_AXIS, -90.0f);
        shader->set_mat4("model", model);
        water->draw_blended(shader);
    }

    void MainController::draw_skybox() const {
        const auto shader = m_resources->shader("skybox");
        shader->use();
        m_fog->apply_to_shader(shader);
        const engine::resources::Skybox *skybox_cube = m_resources->skybox(m_is_day ? m_active_daytime_skybox : m_active_nighttime_skybox);
        m_graphics->draw_skybox(shader, skybox_cube);
    }

    void MainController::draw_grass() const {
        const auto *shader = m_resources->shader("grass_shader");
        m_lighting->apply_to_shader(shader);
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time()));
        shader->set_bool("windEnabled", wind_enabled);
        shader->set_float("windIntensity", wind_intensity);

        constexpr std::array grass_positions = {
            #include <coordinates/grass.include>
        };

        const auto matrices = build_instance_matrices(grass_positions, [](const vec3 &pos) {
            return model_matrix(pos, vec3(20.0f), X_AXIS, 180.0f);
        });
        m_resources->model("grass")->draw_instanced(shader, matrices);
    }

    void MainController::draw_fire() const {
        const engine::resources::Model *fire = m_resources->model("fire");
        const engine::resources::Shader *shader = m_resources->shader("fire_shader");
        shader->use();
        shader->set_vec3("viewPos", m_camera->Position);
        shader->set_mat4("projection", m_graphics->projection_matrix());
        shader->set_mat4("view", m_camera->view_matrix());
        shader->set_mat4("model", model_matrix(vec3(12, 20.5, 6.5), vec3(3.1), Y_AXIS, 0.0f));
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time() - m_fire_start_time));
        shader->set_vec3("fireColor", vec3(1.0f, 0.6f, 0.2f));
        shader->set_vec3("glowColor", vec3(1.0f, 0.3f, 0.0f));
        shader->set_float("intensity", 50.0f);
        shader->set_float("flickerSpeed", 5.0f);
        shader->set_float("distortionAmount", 0.1f);
        fire->draw_blended(shader);
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
                    m_fire_start_time = PlatformController::get_time();
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
            wind_enabled = !wind_enabled;
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
            spdlog::info(std::format("\nModel name: {}\nRotation: {}, {}, {}\nTranslation: {}, {}, {}\nScale: {}",
                model_name, rotation.x, rotation.y, rotation.z,
                translation.x, translation.y, translation.z, scale));
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
        m_shadow->setup_depth_shader(m_depth_shader);

        auto draw_depth = [&](const std::string &model_name, const mat4 &model_mat) {
            m_depth_shader->set_mat4("model", model_mat);
            m_resources->model(model_name)->draw(m_depth_shader);
        };

        draw_depth("terrain", mat4(1.0f));
        draw_depth("terrain", model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        draw_depth("campfire", model_matrix(vec3(12.0f, 17.3f, 6.0f)));

        for (const auto &[angle, pos] : LOGS)
            draw_depth("log_seat", model_matrix(pos, vec3(0.04f), Y_AXIS, angle));

        draw_depth("viking_tent", model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f));
        draw_depth("stylized_tent", model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f));

        for (const auto &shroom : SHROOM_POSITIONS)
            draw_depth("shrooms", model_matrix(shroom, vec3(0.19f), X_AXIS, -90.0f));

        draw_depth("grave", model_matrix(vec3(29, 71, 12), vec3(-90, 0, -48), vec3(1.35)));

        m_shadow->setup_depth_instanced_shader(m_depth_instanced_shader);
        draw_trees(m_depth_instanced_shader);

        constexpr std::array grass_positions = {
            #include <coordinates/grass.include>
        };
        const auto grass_matrices = build_instance_matrices(grass_positions, [](const vec3 &pos) {
            return model_matrix(pos, vec3(20.0f), X_AXIS, 180.0f);
        });
        m_resources->model("grass")->draw_instanced(m_depth_instanced_shader, grass_matrices);
    }
}
