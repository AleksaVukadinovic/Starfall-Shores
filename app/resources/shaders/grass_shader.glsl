//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in mat4 instanceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;
out vec4 FragPosLightSpace;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform float time;
uniform bool windEnabled;
uniform float windIntensity;

void main()
{
    vec3 pos = aPos;

    if (windEnabled) {
        float heightFactor = max(-pos.y, 0.0);
        vec3 worldPos = vec3(instanceMatrix * vec4(0.0, 0.0, 0.0, 1.0));
        float phase = worldPos.x * 0.3 + worldPos.z * 0.2;
        float sway = sin(time * 2.0 + phase) * 0.5 + sin(time * 3.7 + phase * 1.3) * 0.3;
        pos.x += sway * heightFactor * windIntensity;
        pos.z += sway * 0.5 * heightFactor * windIntensity;
    }

    FragPos = vec3(instanceMatrix * vec4(pos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(instanceMatrix)));
    Normal = normalMatrix * aNormal;
    TexCoords = aTexCoords;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
out vec4 FragColor;

struct Material {
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D shadowMap;
uniform bool hasNormalMap;
uniform bool shadowsEnabled;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

uniform bool fogEnabled;
uniform float fogIntensity;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 25.0;
    return shadow;
}

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    if (texColor.a < 0.1)
        discard;

    vec3 modifiedColor = texColor.rgb;

    vec3 norm;
    if (hasNormalMap) {
        norm = texture(texture_normal1, TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        norm = normalize(TBN * norm);
    } else {
        norm = normalize(Normal);
    }

    vec3 lightDir = normalize(light.position - FragPos);

    vec3 ambient = light.ambient * modifiedColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * modifiedColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    float shadow = 0.0;
    if (shadowsEnabled)
        shadow = calculateShadow(FragPosLightSpace, norm, lightDir);

    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    if (fogEnabled) {
        float dist = length(viewPos - FragPos);
        float fogRange = max(fogEnd - fogStart, 0.001);
        float normalizedDist = max(dist - fogStart, 0.0) / fogRange;
        float fogFactor = exp(-pow(normalizedDist * fogIntensity, 2.0));
        result = mix(fogColor, result, fogFactor);
    }

    FragColor = vec4(result, texColor.a);
}
