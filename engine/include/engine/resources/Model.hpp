/**
 * @file Model.hpp
 * @brief Defines the Model class that serves as the interface for model rendering.
*/

#pragma once
#include <engine/resources/Mesh.hpp>
#include <engine/util/Geometry.hpp>
#include <algorithm>
#include <utility>

namespace engine::resources {
    class InstanceBuffer;

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
        void draw(const Shader *shader) const;

        /**
        * @brief Draws `instance_count` instances of the model, reading per-instance transforms from `instances`.
        * @param shader The shader to use for drawing.
        * @param instances Instance data created for this model.
        * @param instance_count Number of instances to draw, at most the number of transforms last uploaded to `instances`.
        */
        void draw_instanced(const Shader *shader, const InstanceBuffer &instances, uint32_t instance_count) const;

        /**
        * @brief Destroys the model in the OpenGL context.
        */  
        void destroy() const;

        /**
        * @brief Returns the meshes in the model.
        * @returns The meshes in the model.
        */  
        [[nodiscard]] const std::vector<Mesh> &meshes() const {
            return m_meshes;
        }

        /**
        * @brief Returns the path to the model file from which the model was loaded.
        * @returns The path to the model.
        */  
        [[nodiscard]] const std::filesystem::path &path() const {
            return m_path;
        }

        /**
        * @brief Returns the name of the model by which it can be referenced using the @ref engine::resources::ResourcesController::model function.
        * @returns The name of the model.
        */  
        [[nodiscard]] const std::string &name() const {
            return m_name;
        }

        /**
        * @brief Returns the bounding box of the model in its local space.
        */
        [[nodiscard]] const util::AABB &bounds() const {
            return m_bounds;
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
        util::AABB m_bounds;

        Model() = default;

        /**
        * @brief Constructs a Model object. Used internally by the @ref engine::resources::ResourcesController class. You are not supposed to call this constructor directly from user code.
        * @param meshes The meshes in the model.
        * @param path The path to the model file from which the model was loaded.
        * @param name The name of the model by which it can be referenced using the @ref engine::resources::ResourcesController::model function.
        * @param bounds The bounding box of all the model vertices.
        */  
        Model(std::vector<Mesh> meshes, std::filesystem::path path,
              std::string name, const util::AABB &bounds) : m_meshes(std::move(meshes))
                              , m_path(std::move(path))
                              , m_name(std::move(name))
                              , m_bounds(bounds) {
        }
    };
} // namespace engine


