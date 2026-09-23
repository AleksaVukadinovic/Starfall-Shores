// Cascaded shadow map sampling. Requires frame.glsl.

uniform sampler2DArrayShadow shadowMap;

// Receivers are pushed towards the light by this many texels to avoid shadow acne.
const float NORMAL_OFFSET_TEXELS = 1.5;
const float DEPTH_BIAS = 0.0002;
// Keeps the 3x3 filter kernel inside the chosen cascade.
const float CASCADE_EDGE_MARGIN = 0.01;

// 3x3 taps of hardware 2x2 PCF, which filters like a smooth 4x4 kernel.
float filterShadow(vec3 coords, int cascade)
{
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0).xy);
    float lit = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            lit += texture(shadowMap, vec4(coords.xy + vec2(x, y) * texelSize, float(cascade), coords.z - DEPTH_BIAS));
        }
    }
    return 1.0 - lit / 9.0;
}

// Returns how much fragPos is in shadow: 0 = fully lit, 1 = fully shadowed.
float calculateShadow(vec3 fragPos, vec3 normal)
{
    if (!shadowsEnabled)
        return 0.0;
    vec3 lightDir = normalize(lightPosition - fragPos);
    vec3 facingNormal = dot(normal, lightDir) < 0.0 ? -normal : normal;
    for (int cascade = 0; cascade < cascadeCount; ++cascade) {
        vec3 offsetPos = fragPos + facingNormal * cascadeTexelSizes[cascade] * NORMAL_OFFSET_TEXELS;
        vec3 coords = (lightSpaceMatrices[cascade] * vec4(offsetPos, 1.0)).xyz * 0.5 + 0.5;
        if (all(greaterThan(coords.xy, vec2(CASCADE_EDGE_MARGIN))) && all(lessThan(coords.xy, vec2(1.0 - CASCADE_EDGE_MARGIN))) && coords.z < 1.0)
            return filterShadow(coords, cascade);
    }
    return 0.0;
}
