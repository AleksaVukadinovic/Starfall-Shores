//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
// Position and heading of each fish: maps +Z (forward) to its swimming direction. See engine::graphics::FlockController.
layout (location = 5) in mat4 instanceMatrix;

#include "frame.glsl"

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

// Turns, centers and scales the model into a body facing +Z, `bodyLength` long.
uniform mat4 bodyTransform;
uniform float bodyLength;

const float SWIM_FREQUENCY = 9.0;

// Fish swim with a wave running from the head to the tail; the tail swings the most.
void main()
{
    vec3 body = vec3(bodyTransform * vec4(aPos, 1.0));
    float alongBody = body.z / bodyLength;    // -0.5 at the tail, 0.5 at the nose
    float wave = sin(time * SWIM_FREQUENCY - alongBody * 6.0 + float(gl_InstanceID) * 1.7);
    body.x += wave * bodyLength * 0.08 * smoothstep(0.35, -0.6, alongBody);

    FragPos = vec3(instanceMatrix * vec4(body, 1.0));
    Normal = mat3(instanceMatrix) * mat3(bodyTransform) * aNormal;
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
