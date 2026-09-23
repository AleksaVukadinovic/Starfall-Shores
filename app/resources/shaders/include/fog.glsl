// Requires frame.glsl.

vec3 applyFog(vec3 color, vec3 fragPos)
{
    if (!fogEnabled)
        return color;
    float dist = length(viewPos - fragPos);
    float fogRange = max(fogEnd - fogStart, 0.001);
    float normalizedDist = max(dist - fogStart, 0.0) / fogRange;
    float fogFactor = exp(-pow(normalizedDist * fogIntensity, 2.0));
    return mix(fogColor, color, fogFactor);
}
