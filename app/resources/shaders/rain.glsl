//#shader vertex
#version 330 core

// No vertex attributes: every drop is a 4 vertex triangle strip, placed from gl_InstanceID and the time.

#include "frame.glsl"

out vec2 DropCoords;
out vec3 FragPos;
out float Fade;

uniform vec3 rainVolume;
uniform vec3 fallVelocity;
uniform float dropLength;
uniform float dropWidth;

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

    // Drops move through world space and wrap around a box centred on the camera, so the rain stays put while the camera moves.
    vec3 boxMin = viewPos - rainVolume * 0.5;
    vec3 head = seed * rainVolume + fallVelocity * time;
    head = boxMin + mod(head - boxMin, rainVolume);

    // A streak from the head back along the fall direction, turned to face the camera around its own axis.
    vec3 axis = normalize(fallVelocity);
    vec3 side = normalize(cross(axis, viewPos - head)) * dropWidth;
    vec2 corner = vec2(gl_VertexID & 1, gl_VertexID >> 1);
    FragPos = head - axis * dropLength * (1.0 - corner.y) + side * (corner.x * 2.0 - 1.0);
    DropCoords = vec2(corner.x * 2.0 - 1.0, corner.y);

    // Fade drops out towards the volume edges, where they wrap around, and right in front of the camera.
    vec3 relative = (head - viewPos) / (rainVolume * 0.5);
    Fade = (1.0 - smoothstep(0.6, 1.0, length(relative.xz)))
         * (1.0 - smoothstep(0.7, 1.0, abs(relative.y)))
         * smoothstep(0.3, 1.5, distance(head, viewPos));

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;

#include "frame.glsl"
#include "fog.glsl"

in vec2 DropCoords;
in vec3 FragPos;
in float Fade;

uniform float opacity;

const vec3 WATER_TINT = vec3(0.8, 0.85, 0.9);

void main()
{
    // Drops mostly show the light around them: the sky and the sun or moon, plus nearby lights like the campfire.
    vec3 light = (lightAmbient * 2.0 + lightDiffuse * 0.6) * lightColor;
    for (int i = 0; i < numPointLights; i++) {
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * dist * dist);
        light += pointLights[i].diffuse * attenuation * 2.0;
    }
    vec3 color = applyFog(light * WATER_TINT, FragPos);

    // Soft across the streak, brightest at the head and fading towards the tail.
    float across = 1.0 - DropCoords.x * DropCoords.x;
    float along = smoothstep(0.0, 0.8, DropCoords.y);
    FragColor = vec4(color, opacity * Fade * across * along);
}
