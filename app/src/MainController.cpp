#include <Constants.hpp>
#include <GUIController.hpp>
#include <MainController.hpp>
#include <engine/core/Controller.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/graphics/PostProcessingController.hpp>
#include <engine/graphics/RainController.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <future>

namespace app {
    using vec3 = glm::vec3;
    using mat4 = glm::mat4;
    class MainPlatformEventObserver final : public engine::platform::PlatformEventObserver {
    public:
        void on_mouse_move(engine::platform::MousePosition position) override;
    };

    void MainPlatformEventObserver::on_mouse_move(const engine::platform::MousePosition position) {
        if (const auto gui = engine::core::Controller::get<GUIController>(); gui->is_enabled())
            return;
        const auto camera = engine::core::Controller::get<engine::graphics::GraphicsController>()->camera();
        camera->rotate_camera(position.dx, position.dy);
    }

    void MainController::initialize() {
        engine::graphics::OpenGL::enable_depth_testing();
        const auto platform = get<engine::platform::PlatformController>();
        platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
        platform->set_enable_cursor(false);
        m_graphics = get<engine::graphics::GraphicsController>();
        m_bloom = get<engine::graphics::PostProcessingController>();
        m_bloom->bloom_setup();
        m_fog               = get<FogController>();
        m_graphics->perspective_params().Far = m_fog->far_plane();
        m_resources         = get<engine::resources::ResourcesController>();
        m_basic_shader      = m_resources->shader("basic");
        m_depth_shader      = m_resources->shader("depth_shader");
        m_depth_instanced_shader = m_resources->shader("depth_instanced_shader");
        m_camera            = m_graphics->camera();
        m_is_day           = true;
        m_camera->Position = vec3(5, 27, 17);
        m_camera->Yaw      = -38;
        m_camera->Pitch    = -5;
        m_camera->rotate_camera(0, 0);
        setup_shadow_map();
    }

    bool MainController::loop() {
        if (const auto platform = get<engine::platform::PlatformController>(); platform->key(
                engine::platform::KeyId::KEY_ESCAPE).is_down() && !get<GUIController>()->is_enabled())
            return false;
        return true;
    }

    void MainController::begin_draw() {
        engine::graphics::OpenGL::clear_buffers();
    }

    void MainController::end_draw() {
        get<engine::platform::PlatformController>()->swap_buffers();
    }

    void MainController::set_common_shader_variables(const engine::resources::Shader *shader) const {
        const auto light_position = m_is_day ? LIGHT_POS_DAY : LIGHT_POS_NIGHT;
        const auto ambient        = m_is_day ? vec3(0.2f) : vec3(0.05f);
        const auto diffuse        = m_is_day ? vec3(0.5f) : vec3(0.2f);
        const auto specular       = m_is_day ? vec3(0.1) : vec3(0.05f);
        const float shininess          = m_is_day ? 1024.0f : 2048.0f;
        const auto light_color    = m_is_day ? LIGHT_COLOR_DAY : LIGHT_COLOR_NIGHT;
        shader->use();
        shader->set_vec3("light.position", light_position);
        shader->set_vec3("light.ambient", ambient);
        shader->set_vec3("light.diffuse", diffuse);
        shader->set_vec3("light.specular", specular);
        shader->set_vec3("lightColor", light_color);
        shader->set_float("material.shininess", shininess);
        shader->set_vec3("viewPos", m_camera->Position);
        shader->set_mat4("projection", m_graphics->projection_matrix());
        shader->set_mat4("view", m_camera->view_matrix());
        shader->set_bool("shadowsEnabled", shadows_enabled);
        if (shadows_enabled) {
            shader->set_mat4("lightSpaceMatrix", light_space_matrix());
            shader->set_int("shadowMap", 7);
            engine::graphics::OpenGL::bind_texture_to_unit(m_shadow_map_texture, 7);
        }
        m_fog->apply_to_shader(shader);
    }

    mat4 create_model_matrix(const vec3 &position, const vec3 &scale, const vec3 &rotation_axis, const float rotation_angle) {
        auto model = mat4(1.0f);
        model = rotate(model, glm::radians(rotation_angle), rotation_axis);
        model = translate(model, position);
        model = glm::scale(model, scale);
        return model;
    }

    void MainController::draw() {
        if (shadows_enabled)
            render_shadow_map();
        m_bloom->underwater = m_camera->Position.y < 7.0f;
        m_bloom->prepare_hdr();
        draw_water();
        draw_terrain();
        draw_campfire();
        draw_logs();
        draw_tents();
        draw_forest();
        draw_bushes();
        draw_flowers();
        draw_path();
        draw_mushrooms();
        draw_grave();
        draw_grass();
        draw_test_model();
        if (!m_is_day)
            draw_fire();
        draw_skybox();
        m_bloom->finalize_bloom();
    }

void MainController::draw_forest() const {
        const auto *shader = m_resources->shader("flower_shader");
        set_common_shader_variables(shader);

        using TreeData = std::array<float, 4>;
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
        struct OldTreeData { float x, y, z, scale, rotation_angle, rx, ry, rz; };
        constexpr std::array<OldTreeData, 2> old_trees = {{
            #include <coordinates/old_trees.include>
        }};

        auto build_matrices = [](const auto &data, auto transform_fn) {
            std::vector<mat4> matrices;
            matrices.reserve(data.size());
            for (const auto &entry : data)
                matrices.push_back(transform_fn(entry));
            return matrices;
        };

        auto yellow_f = std::async(std::launch::async, build_matrices, yellow_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), Y_AXIS, 0.0f);
        });
        m_resources->model("yellow_tree")->draw_instanced(shader, yellow_f.get());

        auto green_f = std::async(std::launch::async, build_matrices, green_trees, [&](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, -90.0f);
        });
        m_resources->model("green_tree")->draw_instanced(shader, green_f.get());

        auto tall_f = std::async(std::launch::async, build_matrices, tall_trees, [&](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), Y_AXIS, 0.0f);
        });
        m_resources->model("beech_tree")->draw_instanced(shader, tall_f.get());

        auto pine_f = std::async(std::launch::async, build_matrices, pine_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, -90.0f);
        });
        m_resources->model("pine_tree")->draw_instanced(shader, pine_f.get());

        auto oak_f = std::async(std::launch::async, build_matrices, oak_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, 90.0f);
        });
        m_resources->model("oak_tree")->draw_instanced(shader, oak_f.get());

        auto old_f = std::async(std::launch::async, build_matrices, old_trees, [](const OldTreeData &t) {
            return create_model_matrix(vec3(t.x, t.y, t.z), vec3(t.scale), vec3(t.rx, t.ry, t.rz), t.rotation_angle);
        });
        m_resources->model("old_tree")->draw_instanced(shader, old_f.get());
    }

    void MainController::draw_campfire() const {
        const engine::resources::Model *campfire = m_resources->model("campfire");
        set_common_shader_variables(m_basic_shader);
        m_basic_shader->set_vec3("light.diffuse", m_is_day ? vec3(0.5f) : vec3(5.0f));
        const mat4 model = translate(mat4(1.0f), vec3(12.0f, 17.3f, 6.0f)); // NOLINT
        m_basic_shader->set_mat4("model", model);
        campfire->draw(m_basic_shader);
    }

    void MainController::draw_logs() const {
        const engine::resources::Model *log_seat = m_resources->model("log_seat");
        set_common_shader_variables(m_basic_shader);

        constexpr std::array<std::pair<float, vec3>, 3> logs = {{
            #include <coordinates/logs.include>
        }};

        for (const auto &[rotation_angle, position] : logs) {
            auto model = create_model_matrix(position, vec3(0.04f), Y_AXIS, rotation_angle);
            m_basic_shader->set_mat4("model", model);
            log_seat->draw(m_basic_shader);
        }
    }

    void MainController::draw_tents() const {
        const engine::resources::Model *viking_tent   = m_resources->model("viking_tent");
        const engine::resources::Model *stylized_tent = m_resources->model("stylized_tent");
        set_common_shader_variables(m_basic_shader);

        auto model = create_model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f);
        m_basic_shader->set_mat4("model", model);
        viking_tent->draw(m_basic_shader);

        model = create_model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f);
        m_basic_shader->set_mat4("model", model);
        stylized_tent->draw(m_basic_shader);
    }

    void MainController::draw_bushes() const {
        set_common_shader_variables(m_basic_shader);
        using BushData = std::array<float, 4>;
        auto draw_bush = [&](const engine::resources::Model *bush_model, const BushData &data, const float rotation_angle = 0.0f, const vec3 &rotation_axis = Y_AXIS) {
            const auto model = create_model_matrix(vec3(data[0], data[1], data[2]), vec3(data[3]), rotation_axis, rotation_angle);
            m_basic_shader->set_mat4("model", model);
            bush_model->draw_blended(m_basic_shader);
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

        const auto *bush1 = m_resources->model("bush1");
        const auto *bush2 = m_resources->model("bush2");
        const auto *laurel = m_resources->model("laurel_bush");

        for (const auto &b : bush1_positions) draw_bush(bush1, b, -90.0f, X_AXIS);
        for (const auto &b : bush2_positions) draw_bush(bush2, b);
        for (const auto &b : laurel_positions) draw_bush(laurel, b);
    }

    void MainController::draw_flowers() const {
        const auto *flower_shader = m_resources->shader("flower_shader");
        set_common_shader_variables(flower_shader);

        constexpr std::array white_translations = {
            #include <coordinates/white_flowers.include>
        };
        constexpr std::array red_translations = {
            #include <coordinates/red_flowers.include>
        };
        auto white_matrices_f = std::async(std::launch::async, [&]() {
            std::vector<mat4> white_matrices;
            white_matrices.reserve(white_translations.size());
            for (const auto &t : white_translations)
                white_matrices.emplace_back(create_model_matrix(t, vec3(0.12f), X_AXIS, 90.0f));
            return white_matrices;
        });
        auto red_matrices_f = std::async(std::launch::async, [&]() {
            std::vector<mat4> red_matrices;
            red_matrices.reserve(red_translations.size());
            for (const auto &t : red_translations) {
                red_matrices.emplace_back(create_model_matrix(t, vec3(0.04f), X_AXIS, -90.0f));
            }
            return red_matrices;
        });

        m_resources->model("white_flowers")->draw_instanced(flower_shader, white_matrices_f.get());
        m_resources->model("red_flowers")->draw_instanced(flower_shader, red_matrices_f.get());
    }

    void MainController::draw_path() const {
        const auto *shader = m_resources->shader("flower_shader");
        set_common_shader_variables(shader);

        struct PathData { float rx, ry, rz, tx, ty, tz, scale; };
        constexpr std::array<PathData, 19> path_segments = {{
            #include <coordinates/path_segments.include>
        }};

        std::vector<mat4> matrices;
        matrices.reserve(path_segments.size());
        for (const auto &[rx, ry, rz, tx, ty, tz, s] : path_segments) {
            auto m = mat4(1.0f);
            m = rotate(m, glm::radians(rx), X_AXIS);
            m = rotate(m, glm::radians(ry), Y_AXIS);
            m = rotate(m, glm::radians(rz), Z_AXIS);
            m = translate(m, vec3(tx, ty, tz));
            m = glm::scale(m, vec3(s));
            matrices.push_back(m);
        }
        m_resources->model("path")->draw_instanced(shader, matrices);
    }

    void MainController::draw_mushrooms() const {
        const auto mushroom = m_resources->model("shrooms");
        set_common_shader_variables(m_basic_shader);

        auto draw_mushroom = [&](const vec3 &translation) {
            const auto model = create_model_matrix(translation, vec3(0.19f), X_AXIS, -90.0f);
            m_basic_shader->set_mat4("model", model);
            mushroom->draw(m_basic_shader);
        };

        constexpr std::array shroom_positions = {
            #include <coordinates/shrooms.include>
        };
        for (const auto &shroom : shroom_positions) {
            draw_mushroom(shroom);
        }
    }

    void MainController::draw_terrain() const {
        const engine::resources::Model *terrain = m_resources->model("terrain");
        set_common_shader_variables(m_basic_shader);
        m_basic_shader->set_mat4("model", mat4(1.0f));
        terrain->draw(m_basic_shader);
        set_common_shader_variables(m_basic_shader);
        m_basic_shader->set_mat4("model", create_model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        terrain->draw(m_basic_shader);
    }

    void MainController::draw_water() const {
        const engine::resources::Model *water = m_resources->model("water");
        const engine::resources::Shader *shader = m_resources->shader("water_shader");

        shader->use();
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time()));
        shader->set_vec3("waterColor", m_is_day? WATER_COLOR_DAY: WATER_COLOR_NIGHT);
        shader->set_vec3("lightPos", m_is_day ? LIGHT_POS_DAY : LIGHT_POS_NIGHT);
        shader->set_vec3("viewPos", m_camera->Position);
        shader->set_mat4("projection", m_graphics->projection_matrix());
        shader->set_mat4("view", m_camera->view_matrix());
        shader->set_bool("shadowsEnabled", shadows_enabled);
        if (shadows_enabled) {
            shader->set_mat4("lightSpaceMatrix", light_space_matrix());
            shader->set_int("shadowMap", 7);
            engine::graphics::OpenGL::bind_texture_to_unit(m_shadow_map_texture, 7);
        }
        m_fog->apply_to_shader(shader);

        const auto model = create_model_matrix(vec3(0, 0, 7), vec3(30, 30, 1), X_AXIS, -90.0f);
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

    void MainController::draw_grave() const {
        const engine::resources::Model *grave = m_resources->model("grave");
        set_common_shader_variables(m_basic_shader);
        auto model = mat4(1.0f);
        model = rotate(model, glm::radians(-90.0f), X_AXIS);
        model = rotate(model, glm::radians(-48.0f), Z_AXIS);
        model = translate(model, vec3(29, 71, 12));
        model = scale(model, vec3(1.35));
        m_basic_shader->set_mat4("model", model);
        grave->draw(m_basic_shader);
    }

    void MainController::draw_grass() const {
        const auto *shader = m_resources->shader("grass_shader");
        set_common_shader_variables(shader);
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time()));
        shader->set_bool("windEnabled", wind_enabled);
        shader->set_float("windIntensity", wind_intensity);

        constexpr std::array grass_positions = {
            #include <coordinates/grass.include>
        };

        std::vector<mat4> matrices;
        matrices.reserve(grass_positions.size());
        for (const auto &pos : grass_positions) {
            matrices.push_back(create_model_matrix(pos, vec3(20.0f), X_AXIS, 180.0f));
        }
        m_resources->model("grass")->draw_instanced(shader, matrices);
    }

    void MainController::draw_fire() const {
        const engine::resources::Model *fire = m_resources->model("fire");
        const engine::resources::Shader *shader = m_resources->shader("fire_shader");
        shader->use();
        shader->set_vec3("viewPos", m_camera->Position);
        shader->set_mat4("projection", m_graphics->projection_matrix());
        shader->set_mat4("view", m_camera->view_matrix());
        shader->set_mat4("model", create_model_matrix(vec3(12, 20.5, 6.5), vec3(3.1), Y_AXIS, 0.0f));
        shader->set_float("time", static_cast<float>(engine::platform::PlatformController::get_time() - m_fire_start_time));
        shader->set_vec3("fireColor", vec3(1.0f, 0.6f, 0.2f));
        shader->set_vec3("glowColor", vec3(1.0f, 0.3f, 0.0f));
        shader->set_float("intensity", 50.0f);
        shader->set_float("flickerSpeed", 5.0f);
        shader->set_float("distortionAmount", 0.1f);
        fire->draw_blended(shader);
    }

    void MainController::update() {
        update_camera();
        update_day_night_transition();
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

    void MainController::update_camera() {
        using namespace engine::platform;
        if (const auto gui = get<GUIController>(); gui->is_enabled())
            return;

        const auto platform = get<PlatformController>();
        const float dt      = platform->dt();
        m_camera->MovementSpeed = platform->key(KEY_LEFT_SHIFT).is_down() ? 20 : 7;
        if (platform->key(KEY_W).is_down() || platform->key(KEY_UP).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
        }
        if (platform->key(KEY_S).is_down() || platform->key(KEY_DOWN).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
        }
        if (platform->key(KEY_A).is_down() || platform->key(KEY_LEFT).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
        }
        if (platform->key(KEY_D).is_down() || platform->key(KEY_RIGHT).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
        }
        if (platform->key(KEY_SPACE).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::UP, dt);
        }
        if (platform->key(KEY_P).state() == Key::State::JustPressed) {
            platform->set_enable_cursor(!platform->is_cursor_enabled());
        }
        if (platform->key(KEY_Q).is_down()) {
            m_camera->rotate_camera(-10, 0);
        }
        if (platform->key(KEY_E).is_down()) {
            m_camera->rotate_camera(10, 0);
        }
        if (platform->key(KEY_ESCAPE).state() == Key::State::JustPressed) {
            get<GUIController>()->set_enable(false);
        }
        if (platform->key(KEY_F).state() == Key::State::JustPressed) {
            get<FogController>()->fog_enabled = !get<FogController>()->fog_enabled;
        }
        if (platform->key(KEY_V).state() == Key::State::JustPressed) {
            wind_enabled = !wind_enabled;
        }
        if (platform->key(KEY_R).state() == Key::State::JustPressed) {
            get<RainController>()->rain_enabled = !get<RainController>()->rain_enabled;
        }
    }

    void MainController::draw_test_model() const {
        set_common_shader_variables(m_basic_shader);

        auto draw_with_transform = [&](const std::string &model_name, const vec3 &translation, const vec3 &rotation, const float scale) {
            auto m = mat4(1.0f);
            m = glm::rotate(m, glm::radians(rotation.x), X_AXIS);
            m = glm::rotate(m, glm::radians(rotation.y), Y_AXIS);
            m = glm::rotate(m, glm::radians(rotation.z), Z_AXIS);
            m = glm::translate(m, translation);
            m = glm::scale(m, vec3(scale));
            m_basic_shader->set_mat4("model", m);
            m_resources->model(model_name)->draw(m_basic_shader);
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
        if (m_shadow_map_fbo) {
            engine::graphics::OpenGL::destroy_shadow_map({m_shadow_map_fbo, m_shadow_map_texture});
        }
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

    void MainController::setup_shadow_map() {
        auto [fbo, texture] = engine::graphics::OpenGL::create_shadow_map(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        m_shadow_map_fbo = fbo;
        m_shadow_map_texture = texture;
    }

    glm::mat4 MainController::light_space_matrix() const {
        const auto light_pos = m_is_day ? LIGHT_POS_DAY : LIGHT_POS_NIGHT;
        const auto light_projection = glm::ortho(-SHADOW_ORTHO_SIZE, SHADOW_ORTHO_SIZE, -SHADOW_ORTHO_SIZE, SHADOW_ORTHO_SIZE, SHADOW_NEAR, SHADOW_FAR);
        const auto light_view = glm::lookAt(light_pos, vec3(0.0f, 15.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f));
        return light_projection * light_view;
    }

    void MainController::render_shadow_map() const {
        engine::graphics::OpenGL::begin_shadow_pass(m_shadow_map_fbo, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        render_depth_scene();
        const auto platform = get<engine::platform::PlatformController>();
        engine::graphics::OpenGL::end_shadow_pass(platform->window()->width(), platform->window()->height());
    }

    void MainController::render_depth_scene() const {
        const auto lsm = light_space_matrix();

        m_depth_shader->use();
        m_depth_shader->set_mat4("lightSpaceMatrix", lsm);

        auto draw_depth = [&](const std::string &model_name, const mat4 &model_mat) {
            m_depth_shader->set_mat4("model", model_mat);
            m_resources->model(model_name)->draw(m_depth_shader);
        };

        draw_depth("terrain", mat4(1.0f));
        draw_depth("terrain", create_model_matrix(vec3(-8.15f, 2.3f, -89.5), vec3(0.72f), Y_AXIS, 0.0f));
        draw_depth("campfire", translate(mat4(1.0f), vec3(12.0f, 17.3f, 6.0f)));

        constexpr std::array<std::pair<float, vec3>, 3> logs = {{
            #include <coordinates/logs.include>
        }};
        for (const auto &[angle, pos] : logs)
            draw_depth("log_seat", create_model_matrix(pos, vec3(0.04f), Y_AXIS, angle));

        draw_depth("viking_tent", create_model_matrix(vec3(16, 17, -14), vec3(0.037), Y_AXIS, -20.0f));
        draw_depth("stylized_tent", create_model_matrix(vec3(0, 20, -33), vec3(0.06), Y_AXIS, -128.0f));

        auto draw_mushroom_depth = [&](const vec3 &t) {
            draw_depth("shrooms", create_model_matrix(t, vec3(0.19f), X_AXIS, -90.0f));
        };
        constexpr std::array shroom_positions = {
            #include <coordinates/shrooms.include>
        };
        for (const auto &shroom : shroom_positions)
            draw_mushroom_depth(shroom);

        {
            auto model = mat4(1.0f);
            model = rotate(model, glm::radians(-90.0f), X_AXIS);
            model = rotate(model, glm::radians(-48.0f), Z_AXIS);
            model = translate(model, vec3(29, 71, 12));
            model = scale(model, vec3(1.35));
            draw_depth("grave", model);
        }

        m_depth_instanced_shader->use();
        m_depth_instanced_shader->set_mat4("lightSpaceMatrix", lsm);

        using TreeData = std::array<float, 4>;
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
        struct OldTreeData { float x, y, z, s, rotation_angle, rx, ry, rz; };
        constexpr std::array<OldTreeData, 2> old_trees = {{
            #include <coordinates/old_trees.include>
        }};

        auto build_matrices = [](const auto &data, auto transform_fn) {
            std::vector<mat4> matrices;
            matrices.reserve(data.size());
            for (const auto &entry : data)
                matrices.emplace_back(transform_fn(entry));
            return matrices;
        };

        auto yellow_f = std::async(std::launch::async, build_matrices, yellow_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), Y_AXIS, 0.0f);
        });
        m_resources->model("yellow_tree")->draw_instanced(m_depth_instanced_shader, yellow_f.get());

        auto green_f = std::async(std::launch::async, build_matrices, green_trees, [&](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, -90.0f);
        });
        m_resources->model("green_tree")->draw_instanced(m_depth_instanced_shader, green_f.get());

        auto tall_f = std::async(std::launch::async, build_matrices, tall_trees, [&](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), Y_AXIS, 0.0f);
        });
        m_resources->model("beech_tree")->draw_instanced(m_depth_instanced_shader, tall_f.get());

        auto pine_f = std::async(std::launch::async, build_matrices, pine_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, -90.0f);
        });
        m_resources->model("pine_tree")->draw_instanced(m_depth_instanced_shader, pine_f.get());

        auto oak_f = std::async(std::launch::async, build_matrices, oak_trees, [](const TreeData &t) {
            return create_model_matrix(vec3(t[0], t[1], t[2]), vec3(t[3]), X_AXIS, 90.0f);
        });
        m_resources->model("oak_tree")->draw_instanced(m_depth_instanced_shader, oak_f.get());

        const auto old_m = build_matrices(old_trees, [](const OldTreeData &t) {
            return create_model_matrix(vec3(t.x, t.y, t.z), vec3(t.s), vec3(t.rx, t.ry, t.rz), t.rotation_angle);
        });
        m_resources->model("old_tree")->draw_instanced(m_depth_instanced_shader, old_m);

        constexpr std::array grass_positions = {
            #include <coordinates/grass.include>
        };
        std::vector<mat4> grass_matrices;
        grass_matrices.reserve(grass_positions.size());
        for (const auto &pos : grass_positions)
            grass_matrices.push_back(create_model_matrix(pos, vec3(20.0f), X_AXIS, 180.0f));
        m_resources->model("grass")->draw_instanced(m_depth_instanced_shader, grass_matrices);
    }
}
