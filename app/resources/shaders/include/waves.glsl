// Gerstner waves (GPU Gems 1, chapter 1). Requires frame.glsl. The wave data is set by engine::graphics::WaterController.

#define WAVE_COUNT 6

// Per wave: xy = direction, z = wave number (2 pi / length), w = amplitude.
uniform vec4 waves[WAVE_COUNT];
// How much the waves bunch up into sharp crests; already limited so that crests never fold over.
uniform float choppiness;
uniform float waveSpeed;

const float GRAVITY = 9.81;

float wavePhase(vec4 wave, vec2 xz)
{
    // Deep water dispersion: longer waves travel faster.
    return wave.z * dot(wave.xy, xz) - sqrt(GRAVITY * wave.z) * waveSpeed * time;
}

// A wave needs this many vertices per wavelength to be displaced without visibly changing shape as the grid slides.
const float MIN_VERTICES_PER_WAVE = 4.0;

// Displacement of the calm surface point at xz, for a grid with `vertexSpacing` between vertices. Waves too short for
// the grid are left out of the geometry; they still shape the surface through the per-pixel normals (waveNormal).
vec3 waveDisplacement(vec2 xz, float vertexSpacing)
{
    vec3 displacement = vec3(0.0);
    for (int i = 0; i < WAVE_COUNT; ++i) {
        float verticesPerWave = 6.2831853 / (waves[i].z * vertexSpacing);
        float amplitude = waves[i].w * smoothstep(MIN_VERTICES_PER_WAVE, 2.0 * MIN_VERTICES_PER_WAVE, verticesPerWave);
        float phase = wavePhase(waves[i], xz);
        displacement.xz += choppiness * amplitude * waves[i].xy * cos(phase);
        displacement.y += amplitude * sin(phase);
    }
    return displacement;
}

// Surface normal at xz. Waves that are too small to see at `viewDistance` are faded out, so they don't shimmer.
vec3 waveNormal(vec2 xz, float viewDistance)
{
    vec3 normal = vec3(0.0, 1.0, 0.0);
    for (int i = 0; i < WAVE_COUNT; ++i) {
        float wavesAcross = viewDistance * waves[i].z / 6.2831853;
        float fade = 1.0 - smoothstep(30.0, 120.0, wavesAcross);
        float phase = wavePhase(waves[i], xz);
        float slope = waves[i].z * waves[i].w * fade;
        normal.xz -= waves[i].xy * slope * cos(phase);
        normal.y -= choppiness * slope * sin(phase);
    }
    return normalize(normal);
}
