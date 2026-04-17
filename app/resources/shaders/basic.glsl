//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalMatrix * aNormal;
    TexCoords = aTexCoords;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;
uniform bool hasNormalMap;
uniform bool hasHeightMap;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform vec3 lightColor;

uniform bool fogEnabled;
uniform float fogIntensity;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if (texColor.a < 0.1)
        discard;

    vec3 norm;
    if (hasNormalMap) {
        norm = texture(texture_normal1, TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        norm = normalize(TBN * norm);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(light.position - FragPos);

    vec3 ambient = light.ambient * texColor.rgb * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texColor.rgb * lightColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular * lightColor;

    vec3 result = ambient + diffuse + specular;

    if (fogEnabled) {
        float dist = length(viewPos - FragPos);
        float fogRange = max(fogEnd - fogStart, 0.001);
        float normalizedDist = max(dist - fogStart, 0.0) / fogRange;
        float fogFactor = exp(-pow(normalizedDist * fogIntensity, 2.0));
        result = mix(fogColor, result, fogFactor);
    }

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}