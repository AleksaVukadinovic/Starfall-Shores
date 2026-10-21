//#shader vertex
#version 330 core

// No vertex attributes: every particle is a camera-facing 4 vertex triangle strip, computed from gl_InstanceID and the time.

#include "frame.glsl"

out vec2 QuadCoords;
out vec3 FragPos;
out vec4 ParticleColor;

uniform vec3 emitterPosition;
uniform vec3 emitterExtent;
uniform bool followCamera;
uniform float visibleCount;
uniform float lifetime;
uniform float lifetimeVariation;
uniform vec3 velocity;
uniform vec3 velocityVariation;
uniform vec3 acceleration;
uniform float turbulence;
uniform float turbulenceFrequency;
// x: size at birth, y: size at the end of life.
uniform vec2 size;
uniform vec4 colorStart;
uniform vec4 colorEnd;
uniform float blinkFrequency;

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
    uint id = uint(gl_InstanceID);
    vec3 particle = hash3(id);

    // Every particle repeats its life forever, each with its own length and starting time.
    float life = lifetime * (1.0 - lifetimeVariation * particle.x);
    float cycleTime = time / life + particle.y;
    float age = fract(cycleTime) * life;
    float normalizedAge = age / life;

    // Each new life starts at a new random spot and with a new random velocity.
    uint cycle = uint(floor(cycleTime));
    vec3 spawn = hash3(id * 7919u + cycle * 104729u) * 2.0 - 1.0;
    vec3 launch = hash3(id * 3571u + cycle * 15485863u) * 2.0 - 1.0;
    vec3 swirl = hash3(id * 2357u + 31u) * 6.2831853;

    vec3 origin = followCamera ? viewPos + emitterPosition : emitterPosition;
    vec3 position = origin + spawn * emitterExtent
                  + (velocity + launch * velocityVariation) * age
                  + 0.5 * acceleration * age * age;
    // Swirling motion: a smooth wobble along a path of a few sine waves.
    float phase = age * turbulenceFrequency * 6.2831853;
    position += turbulence * vec3(sin(phase + swirl.x), 0.5 * sin(1.3 * phase + swirl.y), cos(0.7 * phase + swirl.z));
    if (followCamera) {
        vec3 boxMin = origin - emitterExtent;
        position = boxMin + mod(position - boxMin, 2.0 * emitterExtent);
    }

    ParticleColor = mix(colorStart, colorEnd, normalizedAge);
    // Fade in at birth and out before the end of life, and fade in the newest particle while the emitter fades in.
    ParticleColor.a *= smoothstep(0.0, 0.1, normalizedAge) * smoothstep(1.0, 0.8, normalizedAge)
                     * clamp(visibleCount - float(gl_InstanceID), 0.0, 1.0);
    if (blinkFrequency > 0.0)
        ParticleColor.a *= pow(0.5 + 0.5 * sin((time * blinkFrequency + particle.z) * 6.2831853), 4.0);

    // A quad facing the camera: the camera's right and up axes are the first two rows of the view matrix.
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    QuadCoords = vec2(gl_VertexID & 1, gl_VertexID >> 1) * 2.0 - 1.0;
    float halfSize = 0.5 * mix(size.x, size.y, normalizedAge);
    FragPos = position + (cameraRight * QuadCoords.x + cameraUp * QuadCoords.y) * halfSize;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

#include "frame.glsl"
#include "fog.glsl"

in vec2 QuadCoords;
in vec3 FragPos;
in vec4 ParticleColor;

uniform bool additive;
uniform bool lit;

// The light around a lit particle: the sky, the sun or moon, and nearby point lights such as the campfire.
vec3 surroundingLight()
{
    vec3 light = (lightAmbient * 2.0 + lightDiffuse * 0.6) * lightColor;
    for (int i = 0; i < numPointLights; i++) {
        float dist = length(pointLights[i].position - FragPos);
        light += pointLights[i].diffuse / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * dist * dist);
    }
    return light;
}

void main()
{
    // A soft round dot.
    float coverage = smoothstep(1.0, 0.0, length(QuadCoords));
    float alpha = ParticleColor.a * coverage * coverage;
    if (alpha < 0.002)
        discard;
    vec3 color = lit ? ParticleColor.rgb * surroundingLight() : ParticleColor.rgb;

    if (additive) {
        // Added light: fog hides it rather than tinting it. Like any surface, particles brighter than white glow (bloom).
        vec3 light = color * alpha * fogVisibility(FragPos);
        FragColor = vec4(light, 1.0);
        BrightColor = vec4(dot(color, vec3(0.2126, 0.7152, 0.0722)) > 1.0 ? light : vec3(0.0), 1.0);
    } else {
        // Covers what's behind it; also dims the bloom of what's behind it.
        FragColor = vec4(applyFog(color, FragPos), alpha);
        BrightColor = vec4(0.0, 0.0, 0.0, alpha);
    }
}
