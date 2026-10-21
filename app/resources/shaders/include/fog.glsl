// Requires frame.glsl.

// Light the water scatters towards the camera: the color everything fades to underwater. It darkens as the camera dives.
vec3 underwaterScatter()
{
    float cameraDepth = max(waterLevel - viewPos.y, 0.0);
    return underwaterColor * (lightAmbient * 2.0 + lightDiffuse * lightColor) * exp(-cameraDepth * 0.03);
}

// Share of each color channel that survives the way from fragPos to an underwater camera: water absorbs red first.
vec3 underwaterTransmittance(vec3 fragPos)
{
    return exp(-underwaterAbsorption * length(viewPos - fragPos));
}

// How much of a surface at fragPos is still visible through the fog (or the water): 1 = clear, 0 = hidden.
float fogVisibility(vec3 fragPos)
{
    if (cameraUnderwater)
        return dot(underwaterTransmittance(fragPos), vec3(1.0 / 3.0));
    if (!fogEnabled)
        return 1.0;
    float dist = length(viewPos - fragPos);
    float fogRange = max(fogEnd - fogStart, 0.001);
    float normalizedDist = max(dist - fogStart, 0.0) / fogRange;
    return exp(-pow(normalizedDist * fogIntensity, 2.0));
}

vec3 applyFog(vec3 color, vec3 fragPos)
{
    if (cameraUnderwater) {
        vec3 transmittance = underwaterTransmittance(fragPos);
        return color * transmittance + underwaterScatter() * (1.0 - transmittance);
    }
    return mix(fogColor, color, fogVisibility(fragPos));
}
