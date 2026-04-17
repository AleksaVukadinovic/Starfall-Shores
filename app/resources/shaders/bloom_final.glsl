//#shader vertex
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main() {
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}

//#shader fragment
#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;
uniform float bloomStrength;

uniform bool greyscaleEnabled;
uniform float greyscaleStrength;

uniform bool underwaterEnabled;
uniform vec3 underwaterColor;
uniform float underwaterIntensity;

uniform bool rainEnabled;
uniform float rainIntensity;
uniform float rainSpeed;
uniform float rainOpacity;
uniform float rainStreakLength;
uniform float rainTime;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float rainLayer(vec2 uv, float layerScale, float speed, float streakLen) {
    vec2 cellSize = vec2(0.05 / layerScale, streakLen);
    uv.y += rainTime * speed;
    vec2 cell = floor(uv / cellSize);
    vec2 local = fract(uv / cellSize);
    float rnd = hash(cell);
    float xOffset = rnd;
    float brightness = smoothstep(0.0, 0.3, local.y) * smoothstep(1.0, 0.7, local.y);
    float width = 0.08 + rnd * 0.06;
    float streak = smoothstep(xOffset - width, xOffset, local.x) * smoothstep(xOffset + width, xOffset, local.x);
    float visible = step(0.5 - rainIntensity * 0.5, rnd * 0.3);
    return streak * brightness * visible;
}

void main() {
    const float gamma = 1.3;
    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if (bloom) {
        hdrColor += bloomColor * bloomStrength;
    }

    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);

    result = pow(result, vec3(1.0 / gamma));

    if (greyscaleEnabled) {
        float grey = dot(result, vec3(0.2126, 0.7152, 0.0722));
        result = mix(result, vec3(grey), greyscaleStrength);
    }

    if (underwaterEnabled) {
        result = mix(result, result * underwaterColor, underwaterIntensity);
    }

    if (rainEnabled) {
        float r = 0.0;
        r += rainLayer(TexCoords, 1.0, rainSpeed * 2.0, rainStreakLength);
        r += rainLayer(TexCoords + vec2(0.17, 0.33), 1.5, rainSpeed * 2.5, rainStreakLength * 0.8) * 0.7;
        r += rainLayer(TexCoords + vec2(0.41, 0.67), 2.0, rainSpeed * 3.0, rainStreakLength * 0.6) * 0.4;
        r = clamp(r, 0.0, 1.0) * rainOpacity * rainIntensity;
        vec3 rainColor = vec3(0.7, 0.75, 0.85);
        result = mix(result, rainColor, r);
    }

    FragColor = vec4(result, 1.0);
}