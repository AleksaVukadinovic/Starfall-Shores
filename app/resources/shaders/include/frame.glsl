// Per-frame data uploaded once by the Renderer. Mirrors engine::graphics::FrameUniforms (engine/graphics/ShaderInterface.hpp).

#define MAX_POINT_LIGHTS 4
#define MAX_SHADOW_CASCADES 4

struct PointLight {
    vec3 position;
    float constant;
    vec3 diffuse;
    float linear;
    vec3 specular;
    float quadratic;
};

layout (std140) uniform FrameUniforms {
    mat4 projection;
    mat4 view;
    mat4 lightSpaceMatrices[MAX_SHADOW_CASCADES];
    vec4 cascadeTexelSizes;

    vec3 viewPos;
    float time;
    vec3 lightPosition;
    float shininess;
    vec3 lightAmbient;
    int numPointLights;
    vec3 lightDiffuse;
    bool shadowsEnabled;
    vec3 lightSpecular;
    int cascadeCount;
    vec3 lightColor;
    bool fogEnabled;
    vec3 fogColor;
    float fogIntensity;
    float fogStart;
    float fogEnd;
    float wetness;
    float rainIntensity;

    PointLight pointLights[MAX_POINT_LIGHTS];
};
