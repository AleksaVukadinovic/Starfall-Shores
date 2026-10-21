//#shader vertex
#version 330 core
// xy: position relative to the camera, z: grid spacing (unused here; the grid is shared with the water surface).
layout (location = 0) in vec3 aGridVertex;

#include "frame.glsl"

out vec3 FragPos;

uniform vec2 gridOrigin;
uniform float floorHeight;

void main()
{
    FragPos = vec3(gridOrigin.x + aGridVertex.x, floorHeight, gridOrigin.y + aGridVertex.y);
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

in vec3 FragPos;

const vec3 SAND_DARK = vec3(0.52, 0.45, 0.33);
const vec3 SAND_LIGHT = vec3(0.78, 0.70, 0.53);
// Direction the sand ripples run across, and how many ripples there are per world unit.
const vec2 RIPPLE_DIRECTION = vec2(0.8, 0.6);
const float RIPPLE_FREQUENCY = 1.6;

float sandHash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float sandNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(sandHash(i), sandHash(i + vec2(1.0, 0.0)), f.x),
               mix(sandHash(i + vec2(0.0, 1.0)), sandHash(i + vec2(1.0, 1.0)), f.x), f.y);
}

void main()
{
    vec2 xz = FragPos.xz;
    // Sand ripples: parallel waves across the floor, bent by noise so they don't look ruled.
    float warp = sandNoise(xz * 0.15) * 4.0;
    float ripplePhase = (dot(xz, RIPPLE_DIRECTION) + warp) * RIPPLE_FREQUENCY * 6.2831853;
    float ripple = sin(ripplePhase) * 0.5 + 0.5;
    float rippleSlope = cos(ripplePhase) * 0.25;
    vec3 normal = normalize(vec3(-rippleSlope * RIPPLE_DIRECTION.x, 1.0, -rippleSlope * RIPPLE_DIRECTION.y));

    float speckle = sandNoise(xz * 3.0) * 0.5 + sandNoise(xz * 0.4) * 0.5;
    vec3 albedo = mix(SAND_DARK, SAND_LIGHT, speckle * 0.6 + ripple * 0.4);

    vec3 lightDir = normalize(lightPosition - FragPos);
    float shadow = calculateShadow(FragPos, normal);
    float depth = max(waterLevel - FragPos.y, 0.0);
    vec3 ambient = lightAmbient * lightColor * albedo * exp(-depth * 0.04);
    vec3 diffuse = lightDiffuse * lightColor * albedo * max(dot(normal, lightDir), 0.0) * (1.0 - shadow) * underwaterSunlight(FragPos);
    vec3 color = applyFog(ambient + diffuse, FragPos);

    FragColor = vec4(color, 1.0);
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
