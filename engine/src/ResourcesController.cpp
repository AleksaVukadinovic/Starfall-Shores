#include <unordered_set>
#include <utility>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <engine/graphics/OpenGL.hpp>
#include <engine/resources/ResourcesController.hpp>
#include <engine/resources/ShaderCompiler.hpp>
#include <engine/util/Configuration.hpp>
#include <engine/util/Errors.hpp>
#include <spdlog/spdlog.h>

namespace engine::resources {

void ResourcesController::initialize() {
    load_shaders();
    // Models only request their textures; all the images are then decoded in parallel at once.
    m_texture_loader.emplace();
    load_models();
    load_textures();
    m_texture_loader->load_all();
    m_texture_loader.reset();
}

void ResourcesController::terminate() {
    for (const auto &resource: m_models | std::views::values) {
        resource->destroy();
    }
    for (const auto &resource: m_shaders | std::views::values) {
        resource->destroy();
    }
    for (const auto &resource: m_textures | std::views::values) {
        resource->destroy();
    }
    for (const auto &resource: m_sky_boxes | std::views::values) {
        resource->destroy();
    }
}

void ResourcesController::load_shaders() {
    if (!exists(m_shaders_path)) {
        spdlog::error(std::format("[{}]: no {} found to load the shaders from", this->name(), m_shaders_path.string()));
        return;
    }
    for (const auto &shader_path: std::filesystem::directory_iterator(m_shaders_path)) {
        if (!shader_path.is_regular_file() || shader_path.path().extension() != ".glsl") {
            continue; // e.g. the include/ directory with shared shader code
        }
        const auto name = shader_path.path().stem().string();
        shader(name, shader_path);
    }
}

void ResourcesController::load_models() {
    if (!exists(m_models_path)) {
        spdlog::error(std::format("[ResourcesController]: no {} found to load the models from", m_models_path.string()));
        return;
    }
    const auto &config = util::Configuration::config();
    if (!config.contains("resources") || !config["resources"].contains("models")) {
        throw util::EngineError(util::EngineError::Type::ConfigurationError,
                                "No configuration for models in the config.json, please provide the resources config. See the example in the README.md");
    }
    for (const auto &model_entry: config["resources"]["models"].items()) {
        model(model_entry.key());
    }
}

void ResourcesController::load_textures() {
    if (!exists(m_textures_path)) {
        spdlog::warn(std::format("[ResourcesController]: no {} found to load the textures from", m_textures_path.string()));
        return;
    }
    for (const auto &texture_entry: std::filesystem::directory_iterator(m_textures_path)) {
        texture(texture_entry.path().stem().string(), texture_entry.path());
    }
}

class AssimpSceneProcessor {
public:
    /**
     * @brief Processes the meshes in the scene.
     * @returns The meshes in the scene.
     */
    std::vector<Mesh> process_meshes();

    [[nodiscard]] const util::AABB &bounds() const {
        return m_bounds;
    }

    explicit AssimpSceneProcessor(ResourcesController *resources_controller, const aiScene *scene, std::filesystem::path model_path) :
            m_scene(scene), m_model_path(std::move(model_path)), m_resources_controller(resources_controller) {
    }

private:
    void process_node(const aiNode *node);
    void process_mesh(aiMesh *mesh);
    std::vector<Texture *> process_materials(const aiMaterial *material) const;
    void process_material_type(std::vector<Texture *> &textures, const aiMaterial *material, aiTextureType type) const;
    static TextureType assimp_texture_type_to_engine(aiTextureType type);

    std::vector<Mesh> m_meshes;
    util::AABB m_bounds;
    const aiScene *m_scene;
    std::filesystem::path m_model_path;
    ResourcesController *m_resources_controller;
};

Model *ResourcesController::model(const std::string &name) {
    auto &result = m_models[name];
    if (!result) {
        auto &config = util::Configuration::config();
        if (!config["resources"]["models"].contains(name)) {
            throw util::EngineError(util::EngineError::Type::ConfigurationError, std::format(
            "No model ({}) specify in config.json. Please add the model to the config.json.", name));
        }
        const std::filesystem::path model_path = m_models_path / std::filesystem::path(config["resources"]["models"][name]["path"].get<std::string>());
        Assimp::Importer importer;
        int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;
        if (config["resources"]["models"][name].value<bool>("flip_uvs", false)) {
            flags |= aiProcess_FlipUVs;
        }

        spdlog::info(std::format("load_model(name={}, path={})", name, model_path.string()));
        const aiScene *scene = importer.ReadFile(model_path, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            throw util::EngineError(util::EngineError::Type::AssetLoadingError,
                                    std::format("Assimp error while reading model: {} from path {}.", model_path.string(), name));
        }
        AssimpSceneProcessor scene_processor(this, scene, model_path);
        std::vector<Mesh> meshes = scene_processor.process_meshes();
        result = std::make_unique<Model>(Model(std::move(meshes), model_path, name, scene_processor.bounds()));
    }
    return result.get();
}

Texture *ResourcesController::texture(const std::string &name, const std::filesystem::path &path, const TextureType texture_type, const bool flip_uvs) {
    if (const auto it = m_textures.find(name); it != m_textures.end()) {
        return it->second.get();
    }
    spdlog::info(std::format("load_texture(path={})", path.string()));
    return create_texture(name, path, path.stem().string(), texture_type, [path, flip_uvs] { return load_image(path, flip_uvs); },
                          file_size(path));
}

Texture *ResourcesController::create_texture(const std::string &name, const std::filesystem::path &path, const std::string &texture_name,
                                             const TextureType texture_type, TextureLoader::Decoder decode, const std::uintmax_t decode_cost) {
    const uint32_t texture_id = graphics::OpenGL::create_texture();
    if (m_texture_loader) {
        m_texture_loader->enqueue(texture_id, std::move(decode), decode_cost);
    } else {
        graphics::OpenGL::upload_texture(texture_id, decode());
    }
    auto &texture = m_textures[name];
    texture = std::make_unique<Texture>(Texture(texture_id, texture_type, path, texture_name));
    return texture.get();
}

Skybox *ResourcesController::skybox(const std::string &name, const std::filesystem::path &path, const bool flip_uvs) {
    auto &result = m_sky_boxes[name];
    if (!result) {
        const std::filesystem::path skybox_path = path.empty() ? m_skyboxes_path / name : path;
        spdlog::info(std::format("load_skybox(path={})", skybox_path.string()));
        result = std::make_unique<Skybox>(Skybox(graphics::OpenGL::init_skybox_cube(),
                                                 graphics::OpenGL::load_skybox_textures(skybox_path, flip_uvs),
                                                 skybox_path, name));
    }
    return result.get();
}

Shader *ResourcesController::shader(const std::string &name, const std::filesystem::path &path) {
    auto &result = m_shaders[name];
    if (!result) {
        spdlog::info(std::format("load_shader(path={})", path.string()));
        result = std::make_unique<Shader>(ShaderCompiler::compile_from_file(name, path));
    }
    return result.get();
}

Texture *ResourcesController::texture_from_memory(const std::string &name, const uint8_t *data, const int32_t length,
                                                   const TextureType texture_type, const bool flip_uvs) {
    if (const auto it = m_textures.find(name); it != m_textures.end()) {
        return it->second.get();
    }
    spdlog::info(std::format("load_texture_from_memory(name={})", name));
    // The data belongs to the Assimp scene, which is gone by the time a deferred decode runs, so the decoder keeps a copy.
    return create_texture(name, "", name, texture_type,
                          [bytes = std::vector(data, data + length), flip_uvs] { return load_image_from_memory(bytes, flip_uvs); },
                          static_cast<std::uintmax_t>(length));
}

std::vector<Mesh> AssimpSceneProcessor::process_meshes() {
    m_meshes.clear();
    process_node(m_scene->mRootNode);
    return std::move(m_meshes);
}

void AssimpSceneProcessor::process_node(const aiNode *node) {
    for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
        const auto mesh = m_scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh);
    }
    for (uint32_t i = 0; i < node->mNumChildren; ++i) {
        process_node(node->mChildren[i]);
    }
}

void AssimpSceneProcessor::process_mesh(aiMesh *mesh) {
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex{};
        vertex.Position.x = mesh->mVertices[i].x;
        vertex.Position.y = mesh->mVertices[i].y;
        vertex.Position.z = mesh->mVertices[i].z;
        m_bounds.expand(vertex.Position);

        if (mesh->HasNormals()) {
            vertex.Normal.x = mesh->mNormals[i].x;
            vertex.Normal.y = mesh->mNormals[i].y;
            vertex.Normal.z = mesh->mNormals[i].z;
        }

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
            vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;

            vertex.Tangent.x = mesh->mTangents[i].x;
            vertex.Tangent.y = mesh->mTangents[i].y;
            vertex.Tangent.z = mesh->mTangents[i].z;

            vertex.Bitangent.x = mesh->mBitangents[i].x;
            vertex.Bitangent.y = mesh->mBitangents[i].y;
            vertex.Bitangent.z = mesh->mBitangents[i].z;
        }
        vertices.push_back(vertex);
    }

    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];

        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.emplace_back(face.mIndices[j]);
        }
    }

    auto material = m_scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture *> textures = process_materials(material);
    m_meshes.emplace_back(Mesh(vertices, indices, textures));
}

std::vector<Texture *> AssimpSceneProcessor::process_materials(const aiMaterial *material) const {
    std::vector<Texture *> textures;
    // Only the texture types that shaders sample are loaded; decoding the others would only slow down startup.
    // Add aiTextureType_SPECULAR, aiTextureType_HEIGHT, or aiTextureType_METALNESS here once a shader uses them.
    const auto ai_texture_types = {
            aiTextureType_DIFFUSE,
            aiTextureType_BASE_COLOR,
            aiTextureType_NORMALS,
    };

    for (const auto ai_texture_type: ai_texture_types) {
        process_material_type(textures, material, ai_texture_type);
    }
    return textures;
}

void AssimpSceneProcessor::process_material_type(std::vector<Texture *> &textures, const aiMaterial *material,
                                                 const aiTextureType type) const {
    const auto material_count = material->GetTextureCount(type);
    for (uint32_t i = 0; i < material_count; ++i) {
        aiString ai_texture_path_string;
        material->GetTexture(type, i, &ai_texture_path_string);
        const std::string tex_path_str = ai_texture_path_string.C_Str();

        if (const auto *embedded = m_scene->GetEmbeddedTexture(tex_path_str.c_str())) {
            const std::string embedded_name = m_model_path.string() + "_embedded_" + tex_path_str;
            Texture *texture;
            if (embedded->mHeight == 0) {
                texture = m_resources_controller->texture_from_memory(
                    embedded_name,
                    reinterpret_cast<const uint8_t *>(embedded->pcData),
                    static_cast<int32_t>(embedded->mWidth),
                    assimp_texture_type_to_engine(type));
            } else {
                texture = m_resources_controller->texture_from_memory(
                    embedded_name,
                    reinterpret_cast<const uint8_t *>(embedded->pcData),
                    static_cast<int32_t>(embedded->mWidth * embedded->mHeight * 4),
                    assimp_texture_type_to_engine(type));
            }
            textures.emplace_back(texture);
        } else {
            std::filesystem::path texture_path = m_model_path.parent_path() / ai_texture_path_string.C_Str();
            Texture *texture = m_resources_controller->texture(texture_path.string(), texture_path, assimp_texture_type_to_engine(type));
            textures.emplace_back(texture);
        }
    }
}

TextureType AssimpSceneProcessor::assimp_texture_type_to_engine(const aiTextureType type) {
    switch (type) {
        case aiTextureType_DIFFUSE: return TextureType::Diffuse;
        case aiTextureType_SPECULAR: return TextureType::Specular;
        case aiTextureType_HEIGHT: return TextureType::Height;
        case aiTextureType_NORMALS: return TextureType::Normal;
        case aiTextureType_BASE_COLOR: return TextureType::Diffuse;
        case aiTextureType_METALNESS: return TextureType::Specular;
        default: RG_SHOULD_NOT_REACH_HERE("Engine currently doesn't support the aiTextureType: {}", static_cast<int>(type));
    }
}

} // namespace engine
