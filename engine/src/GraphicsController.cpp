
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine/graphics/GraphicsController.hpp>
#include <engine/graphics/OpenGL.hpp>
#include <engine/platform/PlatformController.hpp>
#include <engine/resources/Skybox.hpp>

namespace engine::graphics {

    void GraphicsController::initialize() {
        const int opengl_initialized = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        RG_GUARANTEE(opengl_initialized, "OpenGL failed to init!");

        auto platform               = engine::core::Controller::get<platform::PlatformController>();
        auto handle                 = platform->window()->handle_();
        m_perspective_params.FOV    = glm::radians(m_camera.Zoom);
        m_perspective_params.Width  = static_cast<float>(platform->window()->width());
        m_perspective_params.Height = static_cast<float>(platform->window()->height());
        m_perspective_params.Near   = 0.1f;
        m_perspective_params.Far    = 200.f;

        m_ortho_params.Bottom = 0.0f;
        m_ortho_params.Top    = static_cast<float>(platform->window()->height());
        m_ortho_params.Left   = 0.0f;
        m_ortho_params.Right  = static_cast<float>(platform->window()->width());
        m_ortho_params.Near   = 0.1f;
        m_ortho_params.Far    = 100.0f;
        platform->register_platform_event_observer(
                std::make_unique<GraphicsPlatformEventObserver>(this));
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void) io;
        RG_GUARANTEE(ImGui_ImplGlfw_InitForOpenGL(handle, true), "ImGUI failed to initialize for OpenGL");
        RG_GUARANTEE(ImGui_ImplOpenGL3_Init("#version 330 core"), "ImGUI failed to initialize for OpenGL");
    }

    void GraphicsController::terminate() {
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void GraphicsPlatformEventObserver::on_window_resize(int width, int height) {
        m_graphics->perspective_params().Width  = static_cast<float>(width);
        m_graphics->perspective_params().Height = static_cast<float>(height);

        m_graphics->orthographic_params().Right = static_cast<float>(width);
        m_graphics->orthographic_params().Top   = static_cast<float>(height);
    }

    std::string_view GraphicsController::name() const {
        return "GraphicsController";
    }

    void GraphicsController::begin_gui() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GraphicsController::end_gui() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void GraphicsController::draw_skybox(const resources::Shader *shader, const resources::Skybox *skybox) {
        glm::mat4 view = glm::mat4(glm::mat3(m_camera.view_matrix()));
        shader->use();
        shader->set_mat4("view", view);
        shader->set_mat4("projection", projection_matrix<>());
        CHECKED_GL_CALL(glDepthFunc, GL_LEQUAL);
        CHECKED_GL_CALL(glBindVertexArray, skybox->vao());
        CHECKED_GL_CALL(glActiveTexture, GL_TEXTURE0);
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, skybox->texture());
        CHECKED_GL_CALL(glDrawArrays, GL_TRIANGLES, 0, 36);
        CHECKED_GL_CALL(glBindVertexArray, 0);
        CHECKED_GL_CALL(glDepthFunc, GL_LESS); // set depth function back to default
        CHECKED_GL_CALL(glBindTexture, GL_TEXTURE_CUBE_MAP, 0);
    }
}
