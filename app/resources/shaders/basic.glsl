//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

#include "frame.glsl"

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

uniform mat4 model;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    Normal = normalMatrix * aNormal;
    TexCoords = aTexCoords;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(Normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

#include "frame.glsl"
#include "shadows.glsl"
#include "fog.glsl"
#include "weather.glsl"

struct Material {
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform bool hasNormalMap;
uniform Material material;

void main()
{
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    if (texColor.a < 0.1)
        discard;

    vec3 norm;
    if (hasNormalMap) {
        norm = texture(texture_normal1, TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        norm = normalize(TBN * norm);
    } else {
        norm = normalize(Normal);
    }

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 lightDir = normalize(lightPosition - FragPos);

    float surfaceWet = surfaceWetness(normalize(Normal));
    vec3 albedo = wetAlbedo(texColor.rgb, surfaceWet);

    vec3 ambient = lightAmbient * albedo * lightColor;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = lightDiffuse * diff * albedo * lightColor;

    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = lightSpecular * spec * material.specular * lightColor + wetSpecular(norm, viewDir, lightDir, surfaceWet);

    float shadow = calculateShadow(FragPos, normalize(Normal));

    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);

    for (int i = 0; i < numPointLights; i++) {
        vec3 pLightDir = normalize(pointLights[i].position - FragPos);
        float pDist = length(pointLights[i].position - FragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * pDist + pointLights[i].quadratic * pDist * pDist);
        float pDiff = max(dot(norm, pLightDir), 0.0);
        vec3 pDiffuse = pointLights[i].diffuse * pDiff * albedo;
        vec3 pReflectDir = reflect(-pLightDir, norm);
        float pSpec = pow(max(dot(viewDir, pReflectDir), 0.0), shininess);
        vec3 pSpecular = pointLights[i].specular * pSpec * material.specular;
        result += attenuation * (pDiffuse + pSpecular);
    }

    result = applyFog(result, FragPos);

    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));

    if (brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}
