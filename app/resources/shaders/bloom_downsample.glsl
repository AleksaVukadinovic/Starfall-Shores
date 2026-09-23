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

out vec3 FragColor;

in vec2 TexCoords;

uniform sampler2D source;
uniform vec2 sourceTexelSize;
uniform bool karisAverage;

float karisWeight(vec3 color) {
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    return 1.0 / (1.0 + luma);
}

// 13-tap downsample from Jimenez, "Next Generation Post Processing in Call of Duty: Advanced Warfare" (2014).
// The taps form five overlapping 2x2 boxes, which gives a smooth, alias-free half resolution image.
void main() {
    vec2 t = sourceTexelSize;
    vec3 a = texture(source, TexCoords + vec2(-2.0, 2.0) * t).rgb;
    vec3 b = texture(source, TexCoords + vec2(0.0, 2.0) * t).rgb;
    vec3 c = texture(source, TexCoords + vec2(2.0, 2.0) * t).rgb;
    vec3 d = texture(source, TexCoords + vec2(-2.0, 0.0) * t).rgb;
    vec3 e = texture(source, TexCoords).rgb;
    vec3 f = texture(source, TexCoords + vec2(2.0, 0.0) * t).rgb;
    vec3 g = texture(source, TexCoords + vec2(-2.0, -2.0) * t).rgb;
    vec3 h = texture(source, TexCoords + vec2(0.0, -2.0) * t).rgb;
    vec3 i = texture(source, TexCoords + vec2(2.0, -2.0) * t).rgb;
    vec3 j = texture(source, TexCoords + vec2(-1.0, 1.0) * t).rgb;
    vec3 k = texture(source, TexCoords + vec2(1.0, 1.0) * t).rgb;
    vec3 l = texture(source, TexCoords + vec2(-1.0, -1.0) * t).rgb;
    vec3 m = texture(source, TexCoords + vec2(1.0, -1.0) * t).rgb;

    vec3 center      = (j + k + l + m) * 0.25;
    vec3 topLeft     = (a + b + d + e) * 0.25;
    vec3 topRight    = (b + c + e + f) * 0.25;
    vec3 bottomLeft  = (d + e + g + h) * 0.25;
    vec3 bottomRight = (e + f + h + i) * 0.25;

    if (karisAverage) {
        // Weighting the boxes by inverse luminance stops single very bright pixels from flickering.
        float wc = karisWeight(center) * 0.5;
        float wtl = karisWeight(topLeft) * 0.125;
        float wtr = karisWeight(topRight) * 0.125;
        float wbl = karisWeight(bottomLeft) * 0.125;
        float wbr = karisWeight(bottomRight) * 0.125;
        FragColor = (center * wc + topLeft * wtl + topRight * wtr + bottomLeft * wbl + bottomRight * wbr)
                  / (wc + wtl + wtr + wbl + wbr);
    } else {
        FragColor = center * 0.5 + (topLeft + topRight + bottomLeft + bottomRight) * 0.125;
    }
    FragColor = max(FragColor, 0.0001);
}
