//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;

#include "frame.glsl"
#include "underwater.glsl"

in vec2 TexCoords;

uniform sampler2D sceneDepth;
uniform mat4 inverseViewProjection;
uniform float strength;

const int STEPS = 24;
// Beams farther away than this are lost in the murk anyway.
const float MAX_DISTANCE = 40.0;
const float BRIGHTNESS = 0.08;

// Interleaved gradient noise (Jimenez, 2014): a per-pixel offset for the samples, which hides the banding of few steps.
float interleavedGradientNoise(vec2 pixel)
{
    return fract(52.9829189 * fract(dot(pixel, vec2(0.06711056, 0.00583715))));
}

// Sunbeams under the water, added to the scene. Along the view ray, light is added wherever the sun shines through a
// bright line of the (large, slow) caustic pattern at the surface; the pattern is stretched along the sun direction into
// sheets of light, which look like beams slanting down. The scene depth stops the ray at the first surface.
void main()
{
    float depth = texture(sceneDepth, TexCoords).r;
    vec4 clip = inverseViewProjection * vec4(vec3(TexCoords, depth) * 2.0 - 1.0, 1.0);
    vec3 ray = clip.xyz / clip.w - viewPos;
    float sceneDistance = length(ray);
    vec3 direction = ray / sceneDistance;
    float marchDistance = min(sceneDistance, MAX_DISTANCE);
    float jitter = interleavedGradientNoise(gl_FragCoord.xy);
    vec3 toSun = normalize(lightPosition - viewPos);

    vec3 light = vec3(0.0);
    for (int i = 0; i < STEPS; ++i) {
        float t = (float(i) + jitter) / float(STEPS) * marchDistance;
        vec3 point = viewPos + direction * t;
        float depthBelowSurface = waterLevel - point.y;
        if (depthBelowSurface <= 0.0)
            continue;
        vec2 entry = point.xz + toSun.xz / max(toSun.y, 0.2) * depthBelowSurface;
        float beam = causticLayer(entry * 0.06, time * 0.25);
        // Beams fade with depth, and on the way to the camera the water absorbs red first.
        light += beam * exp(-depthBelowSurface * 0.05) * exp(-underwaterAbsorption * t);
    }
    light *= marchDistance / float(STEPS) * BRIGHTNESS * strength;
    FragColor = vec4(light * lightDiffuse * lightColor, 1.0);
}
