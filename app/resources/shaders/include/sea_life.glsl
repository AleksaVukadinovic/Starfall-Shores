// Lighting of underwater plants and animals. Requires frame.glsl, shadows.glsl, underwater.glsl and fog.glsl.

// Lit color of thin, two-sided geometry like kelp blades and fins, including light that shines through it from behind.
vec3 shadeSeaLife(vec3 albedo, vec3 normal, vec3 fragPos)
{
    vec3 viewDir = normalize(viewPos - fragPos);
    if (dot(normal, viewDir) < 0.0)
        normal = -normal;
    vec3 lightDir = normalize(lightPosition - fragPos);
    float sunlight = underwaterSunlight(fragPos) * (1.0 - calculateShadow(fragPos, normal));
    float facing = max(dot(normal, lightDir), 0.0);
    float translucency = max(dot(-normal, lightDir), 0.0) * 0.35;

    vec3 color = lightAmbient * lightColor * albedo + lightDiffuse * lightColor * albedo * (facing + translucency) * sunlight;
    for (int i = 0; i < numPointLights; i++) {
        vec3 toLight = pointLights[i].position - fragPos;
        float dist = length(toLight);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * dist * dist);
        color += pointLights[i].diffuse * albedo * max(dot(normal, toLight / dist), 0.0) * attenuation;
    }
    return applyFog(color, fragPos);
}
