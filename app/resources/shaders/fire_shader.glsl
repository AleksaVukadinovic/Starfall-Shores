//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;


in vec2 TexCoords;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform float time;
uniform vec3 fireColor = vec3(10.0, 6.0, 2.0);
uniform vec3 glowColor = vec3(10.0, 3.0, 0.0);
uniform float intensity = 1.5;
uniform float flickerSpeed = 5.0;
uniform float distortionAmount = 0.2;

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    vec2 distortedUV = TexCoords;

    float noiseTime = time * flickerSpeed;
    float noise1 = noise(vec2(distortedUV.x + noiseTime * 0.1, distortedUV.y + noiseTime * 0.3));
    float noise2 = noise(vec2(distortedUV.x * 2.0 - noiseTime * 0.2, distortedUV.y * 2.0 + noiseTime * 0.2));

    float distortionFactor = distortedUV.y * distortionAmount;
    distortedUV.x += sin(noiseTime + distortedUV.y * 10.0) * distortionFactor;

    vec4 texColor = texture(texture_diffuse1, distortedUV);
    float flicker = 1.0 + 0.3 * sin(noiseTime) + 0.2 * noise1 + 0.1 * noise2;
    float verticalGradient = 1.0 - (distortedUV.y * 0.5);
    vec3 finalColor = mix(glowColor, fireColor, texColor.r * verticalGradient);

    float baseBrightness = max(0.0, 1.0 - distortedUV.y * 2.0);
    finalColor += glowColor * baseBrightness * 0.5;
    finalColor *= flicker * intensity * texColor.rgb;

    float alpha = texColor.r * flicker;
    alpha = smoothstep(0.1, 0.7, alpha);
    FragColor = vec4(finalColor, alpha);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if( brightness > 1.0)
        BrightColor = vec4(FragColor.rgb , 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}