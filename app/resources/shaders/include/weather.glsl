// Rain and snow effects on surfaces. Requires frame.glsl.

const vec3 SNOW_COLOR = vec3(0.82, 0.85, 0.9);

// How wet a surface is: surfaces facing up collect the most water, walls some, overhangs none.
float surfaceWetness(vec3 normal)
{
    return wetness * smoothstep(-0.2, 0.7, normal.y);
}

// Water darkens the surfaces it soaks into.
vec3 wetAlbedo(vec3 albedo, float surfaceWet)
{
    return albedo * mix(1.0, 0.55, surfaceWet);
}

// The glossy highlight of a thin water film.
vec3 wetSpecular(vec3 normal, vec3 viewDir, vec3 lightDir, float surfaceWet)
{
    vec3 halfway = normalize(viewDir + lightDir);
    return lightColor * lightDiffuse * surfaceWet * 0.6 * pow(max(dot(normal, halfway), 0.0), 80.0);
}

float snowHash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float snowNoise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(snowHash(i), snowHash(i + vec2(1.0, 0.0)), f.x),
               mix(snowHash(i + vec2(0.0, 1.0)), snowHash(i + vec2(1.0, 1.0)), f.x), f.y);
}

// How much snow lies on a surface: it settles on surfaces facing up, above the water. It covers in patches that grow as
// snowCover rises, until the whole surface is white.
float snowCoverage(vec3 normal, vec3 fragPos)
{
    if (snowCover <= 0.0)
        return 0.0;
    float facing = smoothstep(0.35, 0.8, normal.y);
    float aboveWater = smoothstep(waterLevel + 0.1, waterLevel + 0.6, fragPos.y);
    float patches = snowNoise(fragPos.xz * 0.7) * 0.65 + snowNoise(fragPos.xz * 5.0) * 0.35;
    return smoothstep(patches, patches + 0.15, snowCover * 1.15) * facing * aboveWater;
}

// Snow hides the surface under it.
vec3 snowAlbedo(vec3 albedo, float coverage)
{
    return mix(albedo, SNOW_COLOR, coverage);
}

vec2 rippleHash(vec2 cell)
{
    return fract(sin(vec2(dot(cell, vec2(127.1, 311.7)), dot(cell, vec2(269.5, 183.3)))) * 43758.5453);
}

// Rain drops hitting a water surface at worldXZ: xy is the horizontal slope of the expanding rings, z is the brightness
// of the small splash where a drop just landed. Drops land at random spots and times in a grid of cells rather than
// being traced one by one; as many land as it's raining.
vec3 rainImpacts(vec2 worldXZ)
{
    const float CELL_SIZE = 0.6;
    const float RING_SPEED = 0.8;
    const float MAX_RADIUS = 0.5;
    vec2 p = worldXZ / CELL_SIZE;
    vec2 cell = floor(p);
    vec3 impacts = vec3(0.0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            vec2 neighbour = cell + vec2(x, y);
            vec2 random = rippleHash(neighbour);
            float cycle = time * RING_SPEED + random.x;
            if (rippleHash(neighbour + floor(cycle)).y > rainIntensity)
                continue;
            float age = fract(cycle);
            vec2 toCenter = p - (neighbour + random);
            float dist = length(toCenter);
            float ring = dist - age * MAX_RADIUS / CELL_SIZE;
            float wave = sin(ring * 40.0) * smoothstep(0.15, 0.0, abs(ring)) * (1.0 - age);
            impacts.xy += toCenter / max(dist, 0.001) * wave;
            impacts.z += smoothstep(0.06, 0.0, dist) * smoothstep(0.12, 0.0, age);
        }
    }
    return impacts * vec3(rainIntensity, rainIntensity, 1.0);
}
