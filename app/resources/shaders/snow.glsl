//#shader vertex
#version 330 core

// No vertex attributes: every flake is a 4 vertex triangle strip, placed from gl_InstanceID and the time.

#include "frame.glsl"

out vec2 FlakeCoords;
out vec3 FragPos;
out float Fade;

uniform vec3 snowVolume;
uniform vec3 fallVelocity;
uniform float flakeSize;

// Flakes are never drawn smaller than this share of their distance (about a pixel and a half at 1080p), so distant
// flakes don't flicker as they fall between pixels; they get fainter instead.
const float MIN_ANGULAR_SIZE = 0.0012;
const float FLUTTER_AMPLITUDE = 0.35;

vec3 hash3(uint n)
{
    // Integer hash giving three uncorrelated values in [0, 1).
    n = (n << 13u) ^ n;
    n = n * (n * n * 15731u + 789221u) + 1376312589u;
    uvec3 k = n * uvec3(n, n * 16807u, n * 48271u);
    return vec3((k >> 8u) & uvec3(0x7fffffu)) / float(0x7fffff);
}

void main()
{
    vec3 seed = hash3(uint(gl_InstanceID));
    vec3 variation = hash3(uint(gl_InstanceID) ^ 0x9e3779b9u);

    // Flakes move through world space and wrap around a box centred on the camera, so the snow stays put while the camera moves.
    // Each falls at its own speed, and flutters from side to side on its way down.
    vec3 boxMin = viewPos - snowVolume * 0.5;
    vec3 center = seed * snowVolume + fallVelocity * (0.7 + variation.x * 0.6) * time;
    center = boxMin + mod(center - boxMin, snowVolume);
    float phase = variation.y * 6.2831853;
    float frequency = 0.8 + variation.z * 0.8;
    center += vec3(sin(time * frequency + phase), 0.0, cos(time * frequency * 0.7 + phase * 1.3)) * FLUTTER_AMPLITUDE;

    float dist = distance(center, viewPos);
    float size = flakeSize * (0.6 + variation.z * 0.8);
    float radius = max(size, dist * MIN_ANGULAR_SIZE);

    // A disc facing the camera.
    vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 up = vec3(view[0][1], view[1][1], view[2][1]);
    FlakeCoords = vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0 - 1.0;
    FragPos = center + (right * FlakeCoords.x + up * FlakeCoords.y) * radius;

    // Fade flakes out towards the volume edges, where they wrap around, right in front of the camera, and as they reach
    // the water. Flakes widened for distance fade by the area they gained.
    vec3 relative = (center - viewPos) / (snowVolume * 0.5);
    Fade = (1.0 - smoothstep(0.6, 1.0, length(relative.xz)))
         * (1.0 - smoothstep(0.7, 1.0, abs(relative.y)))
         * smoothstep(0.15, 0.6, dist)
         * smoothstep(waterLevel - 0.1, waterLevel + 0.2, center.y)
         * (size * size) / (radius * radius);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;

#include "frame.glsl"
#include "fog.glsl"

in vec2 FlakeCoords;
in vec3 FragPos;
in float Fade;

uniform float opacity;

const vec3 SNOW_TINT = vec3(0.92, 0.95, 1.0);

void main()
{
    float r2 = dot(FlakeCoords, FlakeCoords);
    if (r2 > 1.0)
        discard;

    // Flakes scatter the light around them: the sky and the sun or moon, plus nearby lights like the campfire.
    vec3 light = (lightAmbient * 1.5 + lightDiffuse * 0.8) * lightColor;
    for (int i = 0; i < numPointLights; i++) {
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * dist * dist);
        light += pointLights[i].diffuse * attenuation * 1.5;
    }
    vec3 color = applyFog(light * SNOW_TINT, FragPos);

    // A soft disc.
    FragColor = vec4(color, opacity * Fade * (1.0 - smoothstep(0.2, 1.0, r2)));
}
