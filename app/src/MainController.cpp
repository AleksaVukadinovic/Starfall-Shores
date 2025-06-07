#include <engine/core/Controller.hpp>
#include <engine/graphics/BloomController.hpp>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Model.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <MainController.hpp>
#include <GUIController.hpp>

namespace app {
    engine::resources::ResourcesController *resources;
    engine::graphics::GraphicsController *graphics;
    engine::graphics::BloomController *bloom_controller;
    GUIController gui;
    engine::graphics::Camera *camera;

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
        graphics         = get<engine::graphics::GraphicsController>();
        bloom_controller = get<engine::graphics::BloomController>();
        bloom_controller->bloom_setup();
        resources           = get<engine::resources::ResourcesController>();
        camera              = graphics->camera();
        resources           = get<engine::resources::ResourcesController>();
        gui                 = GUIController();
        const auto platform = get<engine::platform::PlatformController>();
        platform->register_platform_event_observer(std::make_unique<MainPlatformEventObserver>());
        platform->set_enable_cursor(false);
        m_is_day         = true;
        camera->Front    = glm::vec3(0.77, -0.08, -0.6);
        camera->Position = glm::vec3(5, 27, 17);
        camera->Yaw      = -38;
        camera->Pitch    = -5;
    }

    bool MainController::loop() {
        if (const auto platform = get<engine::platform::PlatformController>(); platform->key(
                engine::platform::KeyId::KEY_ESCAPE).is_down())
            return false;
        return true;
    }

    void MainController::begin_draw() {
        engine::graphics::OpenGL::clear_buffers();
    }

    void MainController::end_draw() {
        get<engine::platform::PlatformController>()->swap_buffers();
    }

    void MainController::set_common_shader_variables(const engine::resources::Shader *shader,
                                                     const engine::graphics::Camera *camera,
                                                     const engine::graphics::GraphicsController *graphics) const {
        const auto light_position = m_is_day ? glm::vec3(0.0f, 60.0f, 0.0f) : glm::vec3(12.0f, 25.0f, 6.0f);
        const auto ambient        = m_is_day ? glm::vec3(0.2f) : glm::vec3(0.1f);
        const auto diffuse        = m_is_day ? glm::vec3(0.5f) : glm::vec3(0.3f);
        const auto specular       = m_is_day ? glm::vec3(0.1) : glm::vec3(0.05f);
        const float shininess     = m_is_day ? 1024.0f : 2048.0f;
        const auto light_color    = m_is_day ? glm::vec3(1.0f, 1.0f, 1.0f) : glm::vec3(1.0f, 0.7f, 0.1f);
        shader->use();
        shader->set_vec3("light.position", light_position);
        shader->set_vec3("light.ambient", ambient);
        shader->set_vec3("light.diffuse", diffuse);
        shader->set_vec3("light.specular", specular);
        shader->set_vec3("lightColor", light_color);
        shader->set_float("material.shininess", shininess);
        shader->set_vec3("viewPos", camera->Position);
        shader->set_mat4("projection", graphics->projection_matrix());
        shader->set_mat4("view", camera->view_matrix());
    }

    void MainController::draw() {
        bloom_controller->prepare_hdr();
        draw_water();
        draw_terrain();
        draw_campfire();
        draw_logs();
        draw_tents();
        draw_old_tree();
        draw_forest();
        draw_bushes();
        draw_flowers();
        draw_path();
        draw_mushrooms();
        draw_stones();
        if (!m_is_day)
            draw_fire();
        draw_skybox();
        bloom_controller->finalize_bloom();
    }

    void MainController::draw_forest() const {
        auto *yellow_tree       = resources->model("yellow_tree");
        auto *green_tree        = resources->model("green_tree");
        auto *tall_tree         = resources->model("beech_tree");
        auto *oak_tree          = resources->model("oak_tree");
        auto *pine_tree         = resources->model("pine_tree");
        const auto *tree_shader = resources->shader("basic");

        set_common_shader_variables(tree_shader, camera, graphics);

        auto draw_tree = [&](auto *tree_model, const float x, const float y, const float z, const float scale,
                             const float rotation_angle = 0.0f, const glm::vec3 &rotation_axis = glm::vec3(0, 0, 0)) {
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

        draw_tree(oak_tree, -17, 28, -17, 0.210f, 90.0f, glm::vec3(1, 0, 0));

        for (const auto &pos: pine_trees) {
            draw_tree(pine_tree, pos.x, pos.y, pos.z, 11.0f, -90.0f, glm::vec3(1, 0, 0));
        }
    }

    void MainController::draw_campfire() const {
        engine::resources::Model *campfire               = resources->model("campfire");
        const engine::resources::Shader *campfire_shader = resources->shader("basic");

        set_common_shader_variables(campfire_shader, camera, graphics);
        campfire_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(5.0f));

        constexpr glm::mat4 model = translate(glm::mat4(1.0f), glm::vec3(12.0f, 17.3f, 6.0f));
        campfire_shader->set_mat4("model", model);
        campfire->draw(campfire_shader);
    }

    void MainController::draw_logs() const {
        engine::resources::Model *log_seat               = resources->model("log_seat");
        const engine::resources::Shader *log_seat_shader = resources->shader("basic");

        set_common_shader_variables(log_seat_shader, camera, graphics);
        log_seat_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(4.0f));

        struct log_placement {
            float rotation_angle;
            glm::vec3 position;
        };

        const std::vector<log_placement> logs = {
                {42.0f, glm::vec3(6, 17.5, 2)},
                {155.0f, glm::vec3(-16, 17.5, -9)},
                {-100.0f, glm::vec3(1, 17.5, -26)}
        };

        for (const auto &[rotation_angle, position]: logs) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(rotation_angle), glm::vec3(0, 1, 0));
            model      = translate(model, position);
            model      = scale(model, glm::vec3(0.04));

            log_seat_shader->set_mat4("model", model);
            log_seat->draw(log_seat_shader);
        }
    }

    void MainController::draw_tents() const {
        engine::resources::Model *viking_tent        = resources->model("viking_tent");
        engine::resources::Model *stylized_tent      = resources->model("stylized_tent");
        const engine::resources::Shader *tent_shader = resources->shader("basic");

        set_common_shader_variables(tent_shader, camera, graphics);
        tent_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(2.0f));

        auto model = glm::mat4(1.0f);
        model      = rotate(model, glm::radians(-20.0f), glm::vec3(0, 1, 0));
        model      = translate(model, glm::vec3(16, 17, -14));
        model      = scale(model, glm::vec3(0.037));
        tent_shader->set_mat4("model", model);
        viking_tent->draw(tent_shader);

        model = glm::mat4(1.0f);
        model = rotate(model, glm::radians(-128.0f), glm::vec3(0, 1, 0));
        model = translate(model, glm::vec3(0, 20, -33));
        model = scale(model, glm::vec3(0.06));
        tent_shader->set_mat4("model", model);
        stylized_tent->draw(tent_shader);
    }

    void MainController::draw_old_tree() const {
        engine::resources::Model *old_tree               = resources->model("old_tree");
        const engine::resources::Shader *old_tree_shader = resources->shader("basic");

        set_common_shader_variables(old_tree_shader, camera, graphics);

        auto model = glm::mat4(1.0f);
        model      = rotate(model, glm::radians(3.0f), glm::vec3(0, 0, 1));
        model      = translate(model, glm::vec3(65, 40, -39));
        model      = scale(model, glm::vec3(0.04));

        old_tree_shader->set_mat4("model", model);

        old_tree->draw(old_tree_shader);
    }

    void MainController::draw_bushes() const {
        const auto bush1       = resources->model("bush1");
        const auto bush2       = resources->model("bush2");
        const auto laurel_bush = resources->model("laurel_bush");
        auto bush_shader       = resources->shader("basic");

        set_common_shader_variables(bush_shader, camera, graphics);

        auto draw_model = [bush_shader](engine::resources::Model *model, const glm::mat4 &transform) {
            bush_shader->set_mat4("model", transform);
            model->draw_blended(bush_shader);
        };

        auto draw_bush1 = [&](const glm::vec3 &translation, const float scale) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            model      = translate(model, translation);
            model      = glm::scale(model, glm::vec3(scale));
            draw_model(bush1, model);
        };

        auto draw_simple = [&](engine::resources::Model *model, const glm::vec3 &translation, const float scale) {
            auto m = glm::mat4(1.0f);
            m      = translate(m, translation);
            m      = glm::scale(m, glm::vec3(scale));
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
        engine::resources::Model *white_flowers        = resources->model("flowers2");
        const engine::resources::Shader *flower_shader = resources->shader("flower_shader");

        constexpr std::array translations = {
            // formatter: off
            #include <white_flowers.include>
            // formatter: off
        };

        constexpr unsigned int row_count = 2;
        constexpr unsigned int col_count = 10;
        constexpr unsigned int amount   = row_count * col_count + translations.size();

        std::vector<glm::mat4> model_matrices;
        model_matrices.reserve(amount);

        for (unsigned int row = 0; row < row_count; row++) {
            const float x = (row == 0) ? 40.0f : 44.0f;

            for (unsigned int col = 0; col < col_count; col++) {
                auto model = glm::mat4(1.0f);
                model = translate(model, glm::vec3(x, 17.4f, 4.0f * static_cast<float>(col) - 16));
                model = rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
                model = scale(model, glm::vec3(0.12f));
                model_matrices.push_back(model);
            }
        }

        for (const auto& translation : translations) {
            auto model = glm::mat4(1.0f);
            model = rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            model = translate(model, translation);
            model = scale(model, glm::vec3(0.12f));
            model_matrices.push_back(model);
        }

        set_common_shader_variables(flower_shader, camera, graphics);
        flower_shader->set_vec3("light.ambient", m_is_day ? glm::vec3(0.2f) : glm::vec3(0.05f));
        flower_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(0.1f));
        white_flowers->draw_instanced(flower_shader, amount, model_matrices.data());
    }

    void MainController::draw_flowers() const {
        draw_white_flowers();
        draw_red_flowers();
    }

    void MainController::draw_path() const {
        const auto path         = resources->model("path");
        const auto stone_shader = resources->shader("basic");

        set_common_shader_variables(stone_shader, camera, graphics);

        auto draw_path_segment = [&](const glm::vec3 &translation, const float yRotation, const float scale) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
            model      = rotate(model, glm::radians(yRotation), glm::vec3(0, 1, 0));
            model      = rotate(model, glm::radians(15.0f), glm::vec3(0, 0, 1));
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
        const auto mushroom      = resources->model("shrooms");
        const auto shroom_shader = resources->shader("basic");

        set_common_shader_variables(shroom_shader, camera, graphics);

        auto draw_mushroom = [&](const glm::vec3 &translation, const float scale, const float yRotation = 0.0f) {
            auto model = glm::mat4(1.0f);
            model      = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            model      = rotate(model, glm::radians(yRotation), glm::vec3(0, 1, 0));
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
        engine::resources::Model *roses                = resources->model("roses");
        const engine::resources::Shader *flower_shader = resources->shader("flower_shader");

        constexpr std::array translations = {
            // formatter: off
            #include <red_flowers.include>
            // formatter: on
        };

        std::vector<glm::mat4> model_matrices;
        model_matrices.reserve(translations.size());

        for (const auto& translation : translations) {
            auto model = glm::mat4(1.0f);
            model = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
            model = translate(model, translation);
            model = scale(model, glm::vec3(0.04f));
            model_matrices.push_back(model);
        }

        set_common_shader_variables(flower_shader, camera, graphics);
        flower_shader->set_vec3("light.ambient", m_is_day ? glm::vec3(0.2f) : glm::vec3(0.05f));
        flower_shader->set_vec3("light.diffuse", m_is_day ? glm::vec3(0.5f) : glm::vec3(0.1f));
        roses->draw_instanced(flower_shader, model_matrices.size(), model_matrices.data());
    }

    void MainController::draw_terrain() const {
        engine::resources::Model *terrain               = resources->model("terrain");
        const engine::resources::Shader *terrain_shader = resources->shader("basic");
        set_common_shader_variables(terrain_shader, camera, graphics);
        auto model = glm::mat4(1.0f);
        model      = translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        terrain_shader->set_mat4("model", model);
        terrain->draw(terrain_shader);
    }

    void MainController::draw_water() const {
        engine::resources::Model *water_model         = resources->model("water");
        const engine::resources::Shader *water_shader = resources->shader("water_shader");

        const auto lightPos = m_is_day ? glm::vec3(0.0f, 60.0f, 0.0f) : glm::vec3(12.0f, 25.0f, 6.0f);
        water_shader->use();

        const auto current_time = static_cast<float>(engine::platform::PlatformController::getTime());
        water_shader->set_float("time", current_time);

        const glm::vec3 waterColor = m_is_day
                                         ? glm::vec3(0.0f, 0.4f, 0.6f)
                                         : glm::vec3(0.0f, 0.1f, 0.3f);
        water_shader->set_vec3("waterColor", waterColor);
        water_shader->set_vec3("lightPos", lightPos);
        water_shader->set_vec3("viewPos", camera->Position);
        water_shader->set_mat4("projection", graphics->projection_matrix());
        water_shader->set_mat4("view", camera->view_matrix());

        auto model = glm::mat4(1.0f);
        model      = scale(model, glm::vec3(30, 1, 30));
        model      = translate(model, glm::vec3(0, 7, 0));
        model      = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        water_shader->set_mat4("model", model);

        water_model->draw_blended(water_shader);
    }

    void MainController::draw_skybox() const {
        const auto shader = get<engine::resources::ResourcesController>()->shader("skybox");
        engine::resources::Skybox *skybox_cube;
        if (m_is_day)
            skybox_cube = get<engine::resources::ResourcesController>()->skybox(active_daytime_skybox);
        else
            skybox_cube = get<engine::resources::ResourcesController>()->skybox(active_nighttime_skybox);
        get<engine::graphics::GraphicsController>()->draw_skybox(shader, skybox_cube);
    }

    void MainController::draw_stones() const {
        engine::resources::Model *grave               = resources->model("grave");
        const engine::resources::Shader *stone_shader = resources->shader("basic");

        set_common_shader_variables(stone_shader, camera, graphics);

        auto model = glm::mat4(1.0f);
        model      = rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        model      = rotate(model, glm::radians(-48.0f), glm::vec3(0, 0, 1));
        model      = translate(model, glm::vec3(29, 71, 12));
        model      = scale(model, glm::vec3(1.35));
        stone_shader->set_mat4("model", model);
        grave->draw(stone_shader);
    }

    void MainController::draw_fire() {
        engine::resources::Model *fire               = resources->model("fire");
        const engine::resources::Shader *fire_shader = resources->shader("fire_shader");
        fire_shader->use();
        fire_shader->set_vec3("viewPos", camera->Position);

        fire_shader->set_mat4("projection", graphics->projection_matrix());
        fire_shader->set_mat4("view", camera->view_matrix());

        auto model = glm::mat4(1.0f);
        model      = translate(model, glm::vec3(12, 20.5, 6.5));
        model      = scale(model, glm::vec3(3.1));
        fire_shader->set_mat4("model", model);

        static double start_time  = engine::platform::PlatformController::getTime();
        const double current_time = engine::platform::PlatformController::getTime() - start_time;

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
    }

    void MainController::update_camera() {
        if (const auto gui = get<GUIController>(); gui->is_enabled())
            return;

        const auto platform = get<engine::platform::PlatformController>();
        const auto camera   = get<engine::graphics::GraphicsController>()->camera();
        const float dt      = platform->dt();
        if (platform->key(engine::platform::KeyId::KEY_W).is_down() || platform->key(engine::platform::KeyId::KEY_UP).
            is_down()) {
            if (platform->key(engine::platform::KEY_LEFT_SHIFT).is_down())
                camera->MovementSpeed = 20;
            else
                camera->MovementSpeed = 7;
            camera->move_camera(engine::graphics::Camera::Movement::FORWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_S).is_down() || platform->key(engine::platform::KeyId::KEY_DOWN).
            is_down()) {
            if (platform->key(engine::platform::KEY_LEFT_SHIFT).is_down())
                camera->MovementSpeed = 20;
            else
                camera->MovementSpeed = 7;
            camera->move_camera(engine::graphics::Camera::Movement::BACKWARD, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_A).is_down() || platform->key(engine::platform::KeyId::KEY_LEFT).
            is_down()) {
            if (platform->key(engine::platform::KEY_LEFT_SHIFT).is_down())
                camera->MovementSpeed = 20;
            else
                camera->MovementSpeed = 7;
            camera->move_camera(engine::graphics::Camera::Movement::LEFT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_D).is_down() || platform->key(engine::platform::KeyId::KEY_RIGHT)
                                                                               .is_down()) {
            if (platform->key(engine::platform::KEY_LEFT_SHIFT).is_down())
                camera->MovementSpeed = 20;
            else
                camera->MovementSpeed = 7;
            camera->move_camera(engine::graphics::Camera::Movement::RIGHT, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_SPACE).is_down()) {
            if (platform->key(engine::platform::KEY_LEFT_SHIFT).is_down())
                camera->move_camera(engine::graphics::Camera::Movement::DOWN, dt);
            else
                camera->move_camera(engine::graphics::Camera::Movement::UP, dt);
        }
        if (platform->key(engine::platform::KeyId::KEY_P).state() == engine::platform::Key::State::JustPressed) {
            platform->set_enable_cursor(!platform->is_cursor_enabled());
        }
        if (platform->key(engine::platform::KeyId::KEY_N).state() == engine::platform::Key::State::JustPressed) {
            m_is_day = !m_is_day;
        }
        if (platform->key(engine::platform::KeyId::KEY_Q).is_down()) {
            camera->rotate_camera(-10, 0);
        }
        if (platform->key(engine::platform::KeyId::KEY_E).is_down()) {
            camera->rotate_camera(10, 0);
        }
    }
}
