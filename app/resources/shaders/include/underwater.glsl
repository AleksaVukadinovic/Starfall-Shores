// Sunlight under the water: dimmed with depth and focused into caustics by the waves above. Requires frame.glsl.

vec2 causticHash(vec2 cell)
{
    return fract(sin(vec2(dot(cell, vec2(127.1, 311.7)), dot(cell, vec2(269.5, 183.3)))) * 43758.5453);
}

// The edges of an animated Voronoi diagram: a wobbling network of bright lines, like light focused by waves.
float causticLayer(vec2 p, float t)
{
    vec2 cell = floor(p);
    vec2 local = fract(p);
    float nearest = 8.0;
    float secondNearest = 8.0;
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            vec2 offset = vec2(x, y);
            vec2 point = offset + 0.5 + 0.4 * sin(t + 6.2831853 * causticHash(cell + offset)) - local;
            float dist = dot(point, point);
            if (dist < nearest) {
                secondNearest = nearest;
                nearest = dist;
            } else if (dist < secondNearest) {
                secondNearest = dist;
            }
        }
    }
    return exp(-(sqrt(secondNearest) - sqrt(nearest)) * 10.0);
}

// Caustics where two layers of lines cross are brightest, as where waves focus light the most.
float causticPattern(vec2 p, float t)
{
    return causticLayer(p, t) * 0.6 + causticLayer(p * 1.37 + 11.3, t * 1.21) * 0.6;
}

// Multiplier for the sunlight reaching fragPos: 1 above the water; below it, dimmer with depth and patterned by caustics.
float underwaterSunlight(vec3 fragPos)
{
    float depth = waterLevel - fragPos.y;
    if (depth <= 0.0)
        return 1.0;
    // Where the sun ray towards fragPos entered the water.
    vec3 toSun = normalize(lightPosition - fragPos);
    vec2 entry = fragPos.xz + toSun.xz / max(toSun.y, 0.2) * depth;
    // Caustics need a little depth to form, and blur out deep down.
    float focus = smoothstep(0.0, 1.5, depth) * exp(-depth * 0.04);
    float caustics = causticsStrength * focus * causticPattern(entry * 0.35, time * 0.8);
    return exp(-depth * 0.06) * (1.0 + 2.0 * caustics);
}
