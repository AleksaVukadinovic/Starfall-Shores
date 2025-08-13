//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 modifiedPos = aPos;
    float waveX = sin(aPos.x * 2.0 + time * 0.5) * 0.3;
    float waveZ = cos(aPos.z * 2.0 + time * 0.5) * 0.3;
    modifiedPos.y += waveX + waveZ;

    vec3 modifiedNormal = aNormal;
    modifiedNormal.x = aNormal.x - 0.1 * cos(aPos.x * 2.0 + time * 0.5);
    modifiedNormal.z = aNormal.z - 0.1 * sin(aPos.z * 2.0 + time * 0.5);
    modifiedNormal = normalize(modifiedNormal);

    FragPos = vec3(model * vec4(modifiedPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * modifiedNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;
uniform float time;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 waterColor;

void main()
{
    vec2 movingTexCoords = TexCoords;
    movingTexCoords.x += sin(time * 0.05 + TexCoords.y * 10.0) * 0.01;
    movingTexCoords.y += cos(time * 0.05 + TexCoords.x * 10.0) * 0.01;

    vec4 waterTexture = texture(texture_diffuse1, movingTexCoords);

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir), 0.2);

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.8;

    spec *= 0.5 + 0.5 * sin(time * 2.0 + TexCoords.x * 30.0 + TexCoords.y * 30.0);

    vec3 result = waterTexture.rgb * diff + vec3(1.0) * spec * specularStrength;

    float depthFactor = 0.7 + 0.3 * sin(time * 0.1 + TexCoords.x * 5.0 + TexCoords.y * 5.0);
    vec3 finalColor = mix(waterColor, result, depthFactor);

    vec3 normViewDir = normalize(viewPos - FragPos);
    float fresnel = pow(1.0 - max(dot(normViewDir, norm), 0.0), 2.0);
    float alpha = mix(0.8, 1.0, fresnel);

    FragColor = vec4(finalColor, alpha);
}