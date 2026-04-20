//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
out float VertexHeight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec3 pos = aPos;
    float height = clamp(pos.y, 0.0, 1.0);
    float sway = sin(time * 3.0 + pos.y * 4.0) * 0.08 * height * height
               + sin(time * 5.3 + pos.y * 7.0) * 0.04 * height
               + sin(time * 1.7 + pos.x * 3.0) * 0.03 * height;
    pos.x += sway;
    pos.z += sway * 0.6;

    FragPos = vec3(model * vec4(pos, 1.0));
    Normal = aNormal;
    TexCoords = aTexCoords;
    VertexHeight = height;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}

//#shader fragment
#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;
in vec3 FragPos;
in float VertexHeight;

uniform sampler2D texture_diffuse1;
uniform float time;
uniform vec3 fireColor;
uniform vec3 glowColor;
uniform float intensity;
uniform float flickerSpeed;
uniform float distortionAmount;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    vec2 shift = vec2(100.0);
    for (int i = 0; i < 5; i++) {
        v += a * valueNoise(p);
        p = p * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

void main() {
    float t = time * flickerSpeed;
    vec2 uv = TexCoords;

    float n1 = fbm(vec2(uv.x * 3.0 + t * 0.4, uv.y * 4.0 - t * 1.2));
    float n2 = fbm(vec2(uv.x * 5.0 - t * 0.3, uv.y * 6.0 - t * 0.8));
    float n3 = fbm(vec2(uv.x * 8.0 + t * 0.7, uv.y * 3.0 + t * 0.5));

    float heightFactor = uv.y * uv.y;
    uv.x += (n1 - 0.5) * distortionAmount * 2.0 * heightFactor;
    uv.x += sin(t * 2.3 + uv.y * 8.0) * 0.03 * heightFactor;
    uv.y += (n2 - 0.5) * distortionAmount * 0.5;

    vec4 texColor = texture(texture_diffuse1, uv);

    float flicker = 0.7
                  + 0.15 * sin(t * 1.0)
                  + 0.10 * sin(t * 2.7 + 1.3)
                  + 0.08 * sin(t * 4.1 + 2.7)
                  + 0.12 * n1
                  + 0.08 * n3;

    vec3 hotCore  = vec3(1.0, 0.95, 0.8);
    vec3 midFlame = fireColor;
    vec3 tipColor = glowColor * 0.7;

    float heightGradient = 1.0 - uv.y;
    vec3 flameColor;
    if (heightGradient > 0.7)
        flameColor = mix(midFlame, hotCore, (heightGradient - 0.7) / 0.3);
    else if (heightGradient > 0.3)
        flameColor = mix(tipColor, midFlame, (heightGradient - 0.3) / 0.4);
    else
        flameColor = tipColor * heightGradient / 0.3;

    float turbulence = n1 * 0.6 + n2 * 0.3 + n3 * 0.1;
    flameColor = mix(flameColor, hotCore, turbulence * 0.3 * heightGradient);

    float baseBrightness = max(0.0, 1.0 - uv.y * 1.5);
    flameColor += glowColor * baseBrightness * 0.4 * turbulence;
    flameColor *= flicker * intensity * texColor.rgb;

    float emberNoise = fbm(vec2(uv.x * 15.0 + t * 2.0, uv.y * 15.0 - t * 3.0));
    float ember = smoothstep(0.72, 0.78, emberNoise) * heightGradient * 0.6;
    flameColor += hotCore * ember * flicker;

    float alpha = texColor.r * flicker;
    alpha *= smoothstep(0.0, 0.15, heightGradient);
    alpha = smoothstep(0.05, 0.6, alpha);

    FragColor = vec4(flameColor, alpha);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
