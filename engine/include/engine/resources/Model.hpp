/**
 * @file Model.hpp
 * @brief Defines the Model class that serves as the interface for model rendering.
*/

#ifndef MATF_RG_PROJECT_MODEL_HPP
#define MATF_RG_PROJECT_MODEL_HPP
#include <engine/resources/Mesh.hpp>
#include <algorithm>
#include <utility>

namespace engine::resources {
    /**
    * @class Model
    * @brief Represents a model object within the OpenGL context as an array of @ref Mesh objects.
    */
    class Model {
        friend class ResourcesController;

    public:
        /**
        * @brief Draws the model using a given shader by drawing all the meshes in the model.
        * @param shader The shader to use for drawing.
        */  
        void draw(const Shader *shader);

        /**
         * @brief Enables blending
         * @param shader The shader to use for drawing
         */
        void draw_blended(const Shader *shader);

        /**
        * @brief Draws the model using a given shader by drawing all the meshes in the model.
        * @param shader The shader to use for drawing.
        * @param number_of_instances Number of model instances to be drawn
        * @param model_matrices Model matrix for each transition
        */
        void draw_instanced(const Shader *shader, unsigned int number_of_instances, glm::mat4* model_matrices);

        /**
        * @brief Destroys the model in the OpenGL context.
        */  
        void destroy();

        /**
        * @brief Returns the meshes in the model.
        * @returns The meshes in the model.
        */  
        const std::vector<Mesh> &meshes() const {
            return m_meshes;
        }

        /**
        * @brief Returns the path to the model file from which the model was loaded.
        * @returns The path to the model.
        */  
        const std::filesystem::path &path() const {
            return m_path;
        }

        /**
        * @brief Returns the name of the model by which it can be referenced using the @ref engine::resources::ResourcesController::model function.
        * @returns The name of the model.
        */  
        const std::string &name() const {
            return m_name;
        }

    private:
        /**
        * @brief The meshes in the model.
        */                      
        std::vector<Mesh> m_meshes;
        /**
        * @brief The path to the model file from which the model was loaded.
        */  
        std::filesystem::path m_path;
        /**
        * @brief The name of the model by which it can be referenced using the @ref engine::resources::ResourcesController::model function.
        */  
        std::string m_name;

        Model() = default;

        /**
        * @brief Constructs a Model object. Used internally by the @ref engine::resources::ResourcesController class. You are not supposed to call this constructor directly from user code.
        * @param meshes The meshes in the model.
        * @param path The path to the model file from which the model was loaded.
        * @param name The name of the model by which it can be referenced using the @ref engine::resources::ResourcesController::model function.
        */  
        Model(std::vector<Mesh> meshes, std::filesystem::path path,
              std::string name) : m_meshes(std::move(meshes))
                              , m_path(std::move(path))
                              , m_name(std::move(name)) {
        }
    };
} // namespace engine

#endif//MATF_RG_PROJECT_MODEL_HPP
