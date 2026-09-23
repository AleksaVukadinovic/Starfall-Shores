/**
 * @file Mesh.hpp
 * @brief Defines the Mesh class that serves as the interface for mesh rendering and storing processed assimp scenes.
 */

#pragma once

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <engine/resources/Texture.hpp>

namespace engine::resources {
    /**
    * @struct Vertex
    * @brief Represents a vertex in the mesh.
    */
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;

        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    /**
    * @class Mesh
    * @brief Represents a mesh in the model in the OpenGL context.
    *
    * Every texture type is bound to a fixed texture unit (see @ref engine::graphics::texture_unit),
    * so drawing a mesh never has to look up sampler uniforms.
    */
    class Mesh {
        friend class AssimpSceneProcessor;

    public:
        /**
        * @brief Draws the mesh using a given shader. Called by the @ref Model::draw function to draw all the meshes in the model.
        * @param shader The shader to use for drawing.
        */
        void draw(const Shader *shader) const;

        /**
        * @brief Draws `instance_count` instances of the mesh using a vertex array that also sources per-instance attributes.
        * @param shader The shader to use for drawing.
        * @param instanced_vao Vertex array created by @ref InstanceBuffer for this mesh.
        * @param instance_count Number of instances that will be drawn.
        */
        void draw_instanced(const Shader *shader, uint32_t instanced_vao, uint32_t instance_count) const;

        /**
        * @brief Binds the mesh vertex and index buffers to the currently bound vertex array and describes the vertex layout.
        * Used to build additional vertex arrays over the same geometry, e.g. for instancing.
        */
        void attach_vertex_buffers() const;

        /**
        * @brief Destroys the mesh in the OpenGL context.
        */
        void destroy() const;

    private:
        /**
        * @brief Constructs a Mesh object.
        * @param vertices The vertices in the mesh.
        * @param indices The indices in the mesh.
        * @param textures The textures in the mesh. If a mesh has several textures of the same type, the first one is used.
         */
        Mesh(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices,
             const std::vector<Texture *> &textures);

        void bind_textures(const Shader *shader) const;

        static constexpr std::size_t TEXTURE_UNIT_COUNT = 4;

        uint32_t m_vao{0};
        uint32_t m_vbo{0};
        uint32_t m_ebo{0};
        uint32_t m_index_count{0};
        std::array<uint32_t, TEXTURE_UNIT_COUNT> m_texture_ids{};
        bool m_has_normal_map{false};
    };
} // namespace engine
