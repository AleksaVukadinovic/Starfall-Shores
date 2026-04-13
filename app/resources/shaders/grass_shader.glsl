//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 instanceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 view;
uniform mat4 projection;
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
    Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal;
    TexCoords = aTexCoords;
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

uniform sampler2D texture_diffuse1;
uniform vec3 viewPos;
uniform Material material;
uniform Light light;

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

    vec3 modifiedColor = texColor.rgb;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    vec3 ambient = light.ambient * modifiedColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * modifiedColor;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * material.specular;

    vec3 result = ambient + diffuse + specular;

    if (fogEnabled) {
        float dist = length(viewPos - FragPos);
        float fogRange = max(fogEnd - fogStart, 0.001);
        float normalizedDist = max(dist - fogStart, 0.0) / fogRange;
        float fogFactor = exp(-pow(normalizedDist * fogIntensity, 2.0));
        result = mix(fogColor, result, fogFactor);
    }

    FragColor = vec4(result, texColor.a);
}