#include <engine/core/Controller.hpp>
#include <engine/graphics/BloomController.hpp>
#include <engine/graphics/FogController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <MainController.hpp>
#include <Constants.hpp>
#include <GUIController.hpp>

namespace app {
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
        m_graphics = get<engine::graphics::GraphicsController>();
        m_graphics->perspective_params().Far = 250.0f;
        m_bloom = get<engine::graphics::BloomController>();
        m_bloom->bloom_setup();
        m_fog               = get<FogController>();
        m_resources         = get<engine::resources::ResourcesController>();
        m_camera            = m_graphics->camera();
        const auto platform = get<engine::platform::PlatformController>();
        platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
        platform->set_enable_cursor(false);
        m_is_day           = true;
        m_camera->Position = glm::vec3(5, 27, 17);
        m_camera->Yaw      = -38;
        m_camera->Pitch    = -5;
        m_camera->rotate_camera(0, 0);
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
        const auto ambient        = m_is_day ? glm::vec3(0.2f) : glm::vec3(0.1f);
        const auto diffuse        = m_is_day ? glm::vec3(0.5f) : glm::vec3(0.3f);
        const auto specular       = m_is_day ? glm::vec3(0.1) : glm::vec3(0.05f);
        const float shininess     = m_is_day ? 1024.0f : 2048.0f;
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
        shader->set_bool("fogEnabled", m_fog->fog_enabled);
        shader->set_float("fogIntensity", m_fog->fog_intensity);
        shader->set_float("fogStart", m_fog->fog_start);
        shader->set_float("fogEnd", m_fog->fog_end);
        shader->set_vec3("fogColor", m_fog->fog_color);
    }

    void MainController::draw() {
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
        draw_stones();
        if (!m_is_day)
            draw_fire();
        draw_skybox();
        m_bloom->finalize_bloom();
    }

    void MainController::draw_forest() const {
        const auto yellow_tree       = m_resources->model("yellow_tree");
        const auto green_tree        = m_resources->model("green_tree");
        const auto tall_tree         = m_resources->model("beech_tree");
        const auto oak_tree          = m_resources->model("oak_tree");
        const auto pine_tree         = m_resources->model("pine_tree");
        const auto old_tree          = m_resources->model("old_tree");
        const auto tree_shader = m_resources->shader("basic");

        set_common_shader_variables(tree_shader);

        auto draw_tree = [&](auto *tree_model, const float x, const float y, const float z, const float scale,
                             const float rotation_angle = 0.0f, const glm::vec3 &rotation_axis = CENTER) {
            auto model = glm::mat4(1.0f);

            if (rotation_angle != 0.0f)
                model = rotate(model, glm::radians(rotation_angle), rotation_axis);

            model = translate(model, glm::vec3(x, y, z));
            model = glm::scale(model, glm::vec3(scale));
            tree_shader->set_mat4("model", model);
            tree_model->draw(tree_shader);
        };

        // formatter: off
        struct tree_placement {
            float x, y, z, scale;
        };
        constexpr std::array<tree_placement, 18> yellow_trees = {{
            #include <yellow_trees.include>
        }};
        constexpr std::array<tree_placement, 15> green_trees = {{
            #include <green_trees.include>
        }};

        constexpr std::array<tree_placement, 3> tall_trees = {{
            #include <tall_trees.include>
        }};

        constexpr std::array<glm::vec3, 26> pine_trees = {{
            #include <pine_trees.include>
        }};
        // formatter: on

        for (const auto &[x, y, z, scale]: yellow_trees) {
            draw_tree(yellow_tree, x, y, z, scale);
        }

        for (const auto &[x, y, z, scale]: green_trees) {
            draw_tree(green_tree, x, y, z, scale, -90.0f, glm::vec3(1.0, 0, 0));
        }

        for (const auto &[x, y, z, scale]: tall_trees) {
            draw_tree(tall_tree, x, y, z, scale);
        }

        draw_tree(oak_tree, -17, 28, -17, 0.210f, 90.0f, X_AXIS);

        for (const auto &pos: pine_trees) {
            draw_tree(pine_tree, pos.x, pos.y, pos.z, 11.0f, -90.0f, X_AXIS);
        }

        draw_tree(old_tree, 65, 40, -39, 0.04f, 3.0f, Z_AXIS);
    }
    void MainController::draw_campfire() const {
        const engine::resources::Model *campfire               = m_resources->model("campfire");
        const engine::resources::Shader *campfire_shader = m_resources->shader("basic");

        set_common_shader_variables(campfire_shader);
        campfire_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(5.0f));

        const glm::mat4 model = translate(glm::mat4(1.0f), glm::vec3(12.0f, 17.3f, 6.0f));
        campfire_shader->set_mat4("model", model);
        campfire->draw(campfire_shader);
    }

    void MainController::draw_logs() const {
        const engine::resources::Model *log_seat               = m_resources->model("log_seat");
        const engine::resources::Shader *log_seat_shader = m_resources->shader("basic");

        set_common_shader_variables(log_seat_shader);
        log_seat_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(4.0f));

        struct LogPlacement {
            float rotation_angle;
            glm::vec3 position;
        };

        constexpr std::array<LogPlacement, 3> logs = {{
                LogPlacement{42.0f, glm::vec3(6, 17.5, 2)},
                LogPlacement{155.0f, glm::vec3(-16, 17.5, -9)},
                LogPlacement{-100.0f, glm::vec3(1, 17.5, -26)}
        }};

        for (const auto &[rotation_angle, position]: logs) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(rotation_angle), Y_AXIS);
            model      = translate(model, position);
            model      = scale(model, glm::vec3(0.04));

            log_seat_shader->set_mat4("model", model);
            log_seat->draw(log_seat_shader);
        }
    }

    glm::mat4 create_model_matrix(const glm::vec3 &position, const glm::vec3 &scale, const glm::vec3 &rotation_axis, const float rotation_angle) {
        auto model = glm::mat4(1.0f);
        model      = rotate(model, glm::radians(rotation_angle), rotation_axis);
        model      = translate(model, position);
        model      = glm::scale(model, scale);
        return model;
    }

    void MainController::draw_tents() const {
        const engine::resources::Model *viking_tent        = m_resources->model("viking_tent");
        const engine::resources::Model *stylized_tent      = m_resources->model("stylized_tent");
        const engine::resources::Shader *tent_shader = m_resources->shader("basic");

        set_common_shader_variables(tent_shader);
        tent_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(2.0f));

        auto model = create_model_matrix(glm::vec3(16, 17, -14), glm::vec3(0.037), Y_AXIS, -20.0f);
        tent_shader->set_mat4("model", model);
        viking_tent->draw(tent_shader);

        model = create_model_matrix(glm::vec3(0, 20, -33), glm::vec3(0.06), Y_AXIS, -128.0f);
        tent_shader->set_mat4("model", model);
        stylized_tent->draw(tent_shader);
    }

    void MainController::draw_bushes() const {
        const auto bush1       = m_resources->model("bush1");
        const auto bush2       = m_resources->model("bush2");
        const auto laurel_bush = m_resources->model("laurel_bush");
        auto bush_shader       = m_resources->shader("basic");

        set_common_shader_variables(bush_shader);

        auto draw_model = [bush_shader](const engine::resources::Model *model, const glm::mat4 &transform) {
            bush_shader->set_mat4("model", transform);
            model->draw_blended(bush_shader);
        };

        auto draw_bush1 = [&](const glm::vec3 &translation, const float scale) {
            const auto model = create_model_matrix(translation, glm::vec3(scale), X_AXIS, -90.0f);
            draw_model(bush1, model);
        };

        auto draw_simple = [&](const engine::resources::Model *model, const glm::vec3 &translation, const float scale) {
            const auto m = create_model_matrix(translation, glm::vec3(scale), Y_AXIS, 0.0f);
            draw_model(model, m);
        };

        draw_bush1(glm::vec3(-19, -3, 16), 5.0f);
        draw_bush1(glm::vec3(15, 25, 16), 5.0f);
        draw_bush1(glm::vec3(52, -19, 17), 5.0f);
        draw_bush1(glm::vec3(31, -32, 17), 5.0f);
        draw_bush1(glm::vec3(12, -24, 17), 5.0f);
        draw_simple(bush2, glm::vec3(4, 20, -13), 0.3f);
        draw_simple(bush2, glm::vec3(32, 20, 4), 0.3f);
        draw_simple(bush2, glm::vec3(30, 20, 12), 0.3f);
        draw_simple(laurel_bush, glm::vec3(-25, 16, 0), 0.680f);
        draw_simple(laurel_bush, glm::vec3(-25, 16, 12), 0.680f);
        draw_simple(laurel_bush, glm::vec3(-20, 16, 23), 0.680f);
        draw_simple(laurel_bush, glm::vec3(-5, 16, 23), 0.680f);
        draw_simple(laurel_bush, glm::vec3(6, 17, 20), 0.680f);
        draw_simple(laurel_bush, glm::vec3(33, 17, -6), 0.680f);
    }

    void MainController::draw_white_flowers() const {
        const engine::resources::Model *white_flowers        = m_resources->model("flowers2");
        const engine::resources::Shader *flower_shader = m_resources->shader("flower_shader");

        constexpr std::array translations = {
            // formatter: off
            #include <white_flowers.include>
            // formatter: off
        };

        constexpr unsigned int row_count = 2;
        constexpr unsigned int col_count = 10;
        constexpr unsigned int amount    = row_count * col_count + translations.size();

        std::vector<glm::mat4> model_matrices;
        model_matrices.reserve(amount);

        for (uint8_t row = 0; row < row_count; row++) {
            const float x = row == 0 ? 40.0f : 44.0f;

            for (uint8_t col = 0; col < col_count; col++) {
                auto model = glm::mat4(1.0f);
                model      = translate(model, glm::vec3(x, 17.4f, 4.0f * static_cast<float>(col) - 16));
                model      = rotate(model, glm::radians(90.0f), X_AXIS);
                model      = scale(model, glm::vec3(0.12f));
                model_matrices.push_back(model);
            }
        }

        for (const auto &translation: translations) {
            auto model = create_model_matrix(translation, glm::vec3(0.12f), X_AXIS, 90.0f);
            model_matrices.push_back(model);
        }

        set_common_shader_variables(flower_shader);
        flower_shader->set_vec3("light.ambient", m_is_day ? glm::vec3(0.2f) : glm::vec3(0.05f));
        flower_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(0.1f));
        white_flowers->draw_instanced(flower_shader, model_matrices);
    }

    void MainController::draw_flowers() const {
        draw_white_flowers();
        draw_red_flowers();
    }

    void MainController::draw_path() const {
        const auto path         = m_resources->model("path");
        const auto stone_shader = m_resources->shader("basic");

        set_common_shader_variables(stone_shader);

        auto draw_path_segment = [&](const glm::vec3 &translation, const float y_rotation, const float scale) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(90.0f), X_AXIS);
            model      = rotate(model, glm::radians(y_rotation), Y_AXIS);
            model      = rotate(model, glm::radians(15.0f), Z_AXIS);
            model      = translate(model, translation);
            model      = glm::scale(model, glm::vec3(scale));
            stone_shader->set_mat4("model", model);
            path->draw(stone_shader);
        };

        draw_path_segment(glm::vec3(-13, 22, -20), 10.0f, 0.19f);
        draw_path_segment(glm::vec3(-11, 19, -17), -1.0f, 0.19f);
        draw_path_segment(glm::vec3(-6.5, 15, -17.5), 0.0f, 0.19f);
        draw_path_segment(glm::vec3(-1, 12, -17.5), 0.0f, 0.19f);
    }

    void MainController::draw_mushrooms() const {
        const auto mushroom      = m_resources->model("shrooms");
        const auto shroom_shader = m_resources->shader("basic");

        set_common_shader_variables(shroom_shader);

        auto draw_mushroom = [&](const glm::vec3 &translation, const float scale, const float y_rotation = 0.0f) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(-90.0f), X_AXIS);
            model      = rotate(model, glm::radians(y_rotation), Y_AXIS);
            model      = translate(model, translation);
            model      = glm::scale(model, glm::vec3(scale));
            shroom_shader->set_mat4("model", model);
            mushroom->draw(shroom_shader);
        };

        draw_mushroom(glm::vec3(6, 0, 16), 0.19f, -19.0f);
        draw_mushroom(glm::vec3(3, 8, 17), 0.19f);
        draw_mushroom(glm::vec3(12, 19, 17), 0.19f);
        draw_mushroom(glm::vec3(30, 1, 17), 0.19f);
        draw_mushroom(glm::vec3(30, -10, 17), 0.19f);
    }

    void MainController::draw_red_flowers() const {
        const engine::resources::Model *roses                = m_resources->model("roses");
        const engine::resources::Shader *flower_shader = m_resources->shader("flower_shader");

        constexpr std::array translations = {
            // formatter: off
            #include <red_flowers.include>
            // formatter: on
        };

        std::vector<glm::mat4> model_matrices;
        model_matrices.reserve(translations.size());

        for (const auto &translation: translations) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(-90.0f), X_AXIS);
            model      = translate(model, translation);
            model      = scale(model, glm::vec3(0.04f));
            model_matrices.push_back(model);
        }

        set_common_shader_variables(flower_shader);
        flower_shader->set_vec3("light.ambient", m_is_day ? glm::vec3(0.2f) : glm::vec3(0.05f));
        flower_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(0.1f));
        roses->draw_instanced(flower_shader, model_matrices);
    }

    void MainController::draw_terrain() const {
        const engine::resources::Model *terrain               = m_resources->model("terrain");
        const engine::resources::Shader *terrain_shader = m_resources->shader("basic");
        set_common_shader_variables(terrain_shader);
        auto model = glm::mat4(1.0f);
        terrain_shader->set_mat4("model", model);
        terrain->draw(terrain_shader);
    }

    void MainController::draw_water() const {
        const engine::resources::Model *water_model         = m_resources->model("water");
        const engine::resources::Shader *water_shader = m_resources->shader("water_shader");

        const auto light_pos = m_is_day ? LIGHT_POS_DAY : LIGHT_POS_NIGHT;
        water_shader->use();

        const auto current_time = static_cast<float>(engine::platform::PlatformController::get_time());
        water_shader->set_float("time", current_time);

        const glm::vec3 water_color = m_is_day? WATER_COLOR_DAY: WATER_COLOR_NIGHT;
        water_shader->set_vec3("waterColor", water_color);
        water_shader->set_vec3("lightPos", light_pos);
        water_shader->set_vec3("viewPos", m_camera->Position);
        water_shader->set_mat4("projection", m_graphics->projection_matrix());
        water_shader->set_mat4("view", m_camera->view_matrix());
        water_shader->set_bool("fogEnabled", m_fog->fog_enabled);
        water_shader->set_float("fogIntensity", m_fog->fog_intensity);
        water_shader->set_float("fogStart", m_fog->fog_start);
        water_shader->set_float("fogEnd", m_fog->fog_end);
        water_shader->set_vec3("fogColor", m_fog->fog_color);

        auto model = glm::mat4(1.0f);
        model      = scale(model, glm::vec3(30, 1, 30));
        model      = translate(model, glm::vec3(0, 7, 0));
        model      = rotate(model, glm::radians(-90.0f), X_AXIS);
        water_shader->set_mat4("model", model);

        water_model->draw_blended(water_shader);
    }

    void MainController::draw_skybox() const {
        const auto shader = m_resources->shader("skybox");
        shader->use();
        shader->set_bool("fogEnabled", m_fog->fog_enabled);
        shader->set_float("fogIntensity", m_fog->fog_intensity);
        shader->set_float("fogStart", m_fog->fog_start);
        shader->set_float("fogEnd", m_fog->fog_end);
        shader->set_vec3("fogColor", m_fog->fog_color);
        engine::resources::Skybox *skybox_cube;
        if (m_is_day)
            skybox_cube = m_resources->skybox(m_active_daytime_skybox);
        else
            skybox_cube = m_resources->skybox(m_active_nighttime_skybox);
        m_graphics->draw_skybox(shader, skybox_cube);
    }

    void MainController::draw_stones() const {
        const engine::resources::Model *grave               = m_resources->model("grave");
        const engine::resources::Shader *stone_shader = m_resources->shader("basic");

        set_common_shader_variables(stone_shader);

        auto model = glm::mat4(1.0f);
        model      = rotate(model, glm::radians(-90.0f), X_AXIS);
        model      = rotate(model, glm::radians(-48.0f), Z_AXIS);
        model      = translate(model, glm::vec3(29, 71, 12));
        model      = scale(model, glm::vec3(1.35));
        stone_shader->set_mat4("model", model);
        grave->draw(stone_shader);
    }

    void MainController::draw_fire() const {
        const engine::resources::Model *fire               = m_resources->model("fire");
        const engine::resources::Shader *fire_shader = m_resources->shader("fire_shader");
        fire_shader->use();
        fire_shader->set_vec3("viewPos", m_camera->Position);
        fire_shader->set_mat4("projection", m_graphics->projection_matrix());
        fire_shader->set_mat4("view", m_camera->view_matrix());

        const auto model = create_model_matrix(glm::vec3(12, 20.5, 6.5), glm::vec3(3.1), Y_AXIS, 0.0f);
        fire_shader->set_mat4("model", model);

        const double current_time = engine::platform::PlatformController::get_time() - m_fire_start_time;

        fire_shader->set_float("time", static_cast<float>(current_time));
        fire_shader->set_vec3("fireColor", glm::vec3(1.0f, 0.6f, 0.2f));
        fire_shader->set_vec3("glowColor", glm::vec3(1.0f, 0.3f, 0.0f));
        fire_shader->set_float("intensity", 50.0f);
        fire_shader->set_float("flickerSpeed", 5.0f);
        fire_shader->set_float("distortionAmount", 0.1f);

        fire->draw_blended(fire_shader);
    }

    void MainController::update() {
        update_camera();
        update_day_night_transition();
    }

    void MainController::update_day_night_transition() {
        if (const auto platform = get<engine::platform::PlatformController>(); platform->key(engine::platform::KeyId::KEY_N).state() == engine::platform::Key::State::JustPressed) {
            if (!m_day_change_requested) {
                m_day_change_requested = true;
                m_day_change_timer = engine::platform::PlatformController::get_time();
            }
        }

        if (m_day_change_requested) {
            const double current_time = engine::platform::PlatformController::get_time();
            const double elapsed_time = current_time - m_day_change_timer;

            if (const auto transition_progress = static_cast<float>(elapsed_time / DAY_CHANGE_DELAY);
                transition_progress >= 1.0f) {
                m_is_day = !m_is_day;
                m_current_exposure = m_is_day ? DAY_EXPOSURE : NIGHT_EXPOSURE;
                m_day_change_requested = false;
                if (!m_is_day)
                    m_fire_start_time = engine::platform::PlatformController::get_time();
            } else {
                const float start_exposure = m_is_day ? DAY_EXPOSURE : NIGHT_EXPOSURE;
                const float target_exposure = m_is_day ? NIGHT_EXPOSURE : DAY_EXPOSURE;
                m_current_exposure = start_exposure + (target_exposure - start_exposure) * transition_progress;
            }
            m_bloom->exposure = m_current_exposure;
        }
    }

    void MainController::update_camera() const {
        if (const auto gui = get<GUIController>(); gui->is_enabled())
            return;

        const auto platform = get<engine::platform::PlatformController>();
        const float dt      = platform->dt();
        m_camera->MovementSpeed = platform->key(engine::platform::KEY_LEFT_SHIFT).is_down() ? 20 : 7;
        if (platform->key(engine::platform::KeyId::KEY_W).is_down() ||
            platform->key(engine::platform::KeyId::KEY_UP).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_S).is_down() ||
            platform->key(engine::platform::KeyId::KEY_DOWN).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_A).is_down() || platform->key(engine::platform::KeyId::KEY_LEFT).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_D).is_down()
            || platform->key(engine::platform::KeyId::KEY_RIGHT).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_SPACE).is_down()) {
            m_camera->move_camera(engine::graphics::Camera::Movement::UP, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_P).state() == engine::platform::Key::State::JustPressed) {
            platform->set_enable_cursor(!platform->is_cursor_enabled());
        }
        if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
            m_camera->rotate_camera(-10, 0);
        }
        if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
            m_camera->rotate_camera(10, 0);
        }
        if (platform->key(engine::platform::KeyId::KEY_ESCAPE).state() == engine::platform::Key::State::JustPressed) {
            get<GUIController>()->set_enable(false);
        }
        if (platform->key(engine::platform::KeyId::KEY_F).state() == engine::platform::Key::State::JustPressed) {
            get<FogController>()->fog_enabled = !get<FogController>()->fog_enabled;
        }
    }

    void MainController::set_skybox(const std::string &new_skybox, const bool is_daytime_skybox) {
        if (is_daytime_skybox) {
            m_active_daytime_skybox = new_skybox;
        } else {
            m_active_nighttime_skybox = new_skybox;
        }
    }
}
