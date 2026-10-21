//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in mat4 instanceMatrix;

#include "frame.glsl"

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Kelp swaying in the current. The model is 1 unit tall with its root at y = 0: the higher a point, the farther it sways.
void main()
{
    float phase = dot(instanceMatrix[3].xz, vec2(0.37, 0.23));
    float bend = aPos.y * aPos.y;
    vec3 position = aPos;
    position.x += sin(time * 0.9 + phase) * 0.12 * bend + sin(time * 1.7 + phase * 1.3 + aPos.y * 3.0) * 0.03 * aPos.y;
    position.z += cos(time * 0.7 + phase) * 0.08 * bend;

    FragPos = vec3(instanceMatrix * vec4(position, 1.0));
    Normal = mat3(transpose(inverse(instanceMatrix))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

#include "frame.glsl"
#include "shadows.glsl"
#include "underwater.glsl"
#include "fog.glsl"
#include "sea_life.glsl"

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{
    vec3 albedo = texture(texture_diffuse1, TexCoords).rgb;
    FragColor = vec4(shadeSeaLife(albedo, normalize(Normal), FragPos), 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
