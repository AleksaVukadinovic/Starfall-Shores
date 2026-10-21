//#shader vertex
#version 330 core
// xy: position relative to the camera, z: distance to the neighbouring grid vertices.
layout (location = 0) in vec3 aGridVertex;

#include "frame.glsl"
#include "waves.glsl"

out vec3 FragPos;
out vec2 WaveCoords;
out float CrestHeight;

uniform vec2 gridOrigin;
uniform float totalAmplitude;

void main()
{
    vec2 xz = gridOrigin + aGridVertex.xy;
    vec3 displacement = waveDisplacement(xz, aGridVertex.z);
    FragPos = vec3(xz.x, waterLevel, xz.y) + displacement;
    WaveCoords = xz;
    CrestHeight = displacement.y / max(totalAmplitude, 0.0001);
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

#include "frame.glsl"
#include "waves.glsl"
#include "shadows.glsl"
#include "fog.glsl"
#include "weather.glsl"

in vec3 FragPos;
in vec2 WaveCoords;
in float CrestHeight;

uniform samplerCube environmentMap;
uniform vec2 windDirection;
uniform vec3 deepColor;
uniform vec3 shallowColor;
uniform float foamAmount;
uniform float opacity;

// Small ripples on top of the waves; they only affect lighting, and fade out with distance.
vec2 detailSlope(vec2 xz, float viewDistance)
{
    const float LENGTHS[4] = float[](0.9, 0.6, 0.4, 0.25);
    const float ANGLES[4] = float[](0.3, -0.5, 0.9, -1.2);
    vec2 slope = vec2(0.0);
    for (int i = 0; i < 4; ++i) {
        float c = cos(ANGLES[i]), s = sin(ANGLES[i]);
        vec2 direction = vec2(c * windDirection.x - s * windDirection.y, s * windDirection.x + c * windDirection.y);
        float k = 6.2831853 / LENGTHS[i];
        float phase = k * dot(direction, xz) - sqrt(GRAVITY * k) * waveSpeed * time;
        slope += direction * cos(phase) * 0.06;
    }
    return slope * (1.0 - smoothstep(5.0, 40.0, viewDistance));
}

// The surface seen from underwater (normal facing down, towards the camera). Light leaving the water bends away from the
// normal, so the whole sky is squeezed into a bright circle above the viewer (Snell's window); outside it, the surface
// reflects everything back (total internal reflection), mirroring the dark water below.
vec3 surfaceFromBelow(vec3 normal, vec3 viewDir)
{
    const float WATER_TO_AIR = 1.33;
    vec3 intoAir = refract(-viewDir, normal, WATER_TO_AIR);
    if (dot(intoAir, intoAir) < 0.0001)
        return underwaterScatter() * 0.6;
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    return mix(texture(environmentMap, intoAir).rgb * 1.2, underwaterScatter(), fresnel);
}

void main()
{
    float viewDistance = distance(viewPos, FragPos);
    vec3 impacts = rainImpacts(WaveCoords) * (1.0 - smoothstep(15.0, 50.0, viewDistance));

    vec3 normal = waveNormal(WaveCoords, viewDistance);
    normal.xz -= detailSlope(WaveCoords, viewDistance) + impacts.xy * 0.6;
    normal = normalize(normal);
    // Seen from below, the surface faces down.
    if (viewPos.y < waterLevel)
        normal = -normal;

    vec3 viewDir = normalize(viewPos - FragPos);
    if (viewPos.y < waterLevel) {
        FragColor = vec4(applyFog(surfaceFromBelow(normal, viewDir), FragPos), 1.0);
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec3 lightDir = normalize(lightPosition - FragPos);
    float shadow = calculateShadow(FragPos, normal);
    vec3 sunLight = lightDiffuse * lightColor * (1.0 - shadow);
    vec3 ambient = lightAmbient * lightColor;

    // Schlick's approximation: water reflects little when looked at from above and almost everything at grazing angles.
    float fresnel = 0.02 + 0.98 * pow(1.0 - max(dot(normal, viewDir), 0.0), 5.0);
    vec3 reflected = reflect(-viewDir, normal);
    reflected.y = abs(reflected.y);
    vec3 sky = texture(environmentMap, reflected).rgb;

    // Light scattered inside the water: wave crests are thinner, so more light gets through them.
    vec3 body = mix(deepColor, shallowColor, clamp(CrestHeight * 0.5 + 0.5, 0.0, 1.0));
    vec3 color = mix(body * (ambient * 3.0 + sunLight * max(dot(normal, lightDir), 0.0)), sky, fresnel);

    // Sun glints are much brighter than 1, so they feed the bloom.
    vec3 specular = sunLight * pow(max(dot(normal, normalize(viewDir + lightDir)), 0.0), 512.0) * 6.0;
    for (int i = 0; i < numPointLights; i++) {
        vec3 pointLightDir = normalize(pointLights[i].position - FragPos);
        float dist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * dist + pointLights[i].quadratic * dist * dist);
        specular += pointLights[i].specular * attenuation * pow(max(dot(normal, normalize(viewDir + pointLightDir)), 0.0), 256.0) * 4.0;
    }

    float foam = clamp(foamAmount * smoothstep(0.55, 1.0, CrestHeight) + impacts.z, 0.0, 1.0);
    color = mix(color + specular, (ambient + sunLight) * 0.9, foam);
    color = applyFog(color, FragPos);
    float alpha = mix(opacity, 1.0, max(fresnel, foam));

    FragColor = vec4(color, alpha);
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    BrightColor = brightness > 1.0 ? vec4(color, alpha) : vec4(0.0, 0.0, 0.0, alpha);
}
