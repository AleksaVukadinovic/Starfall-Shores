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
uniform float filterRadius;

// 3x3 tent filter; the result is blended additively onto the next larger mip.
void main() {
    float x = filterRadius;
    float y = filterRadius;

    vec3 a = texture(source, TexCoords + vec2(-x, y)).rgb;
    vec3 b = texture(source, TexCoords + vec2(0.0, y)).rgb;
    vec3 c = texture(source, TexCoords + vec2(x, y)).rgb;
    vec3 d = texture(source, TexCoords + vec2(-x, 0.0)).rgb;
    vec3 e = texture(source, TexCoords).rgb;
    vec3 f = texture(source, TexCoords + vec2(x, 0.0)).rgb;
    vec3 g = texture(source, TexCoords + vec2(-x, -y)).rgb;
    vec3 h = texture(source, TexCoords + vec2(0.0, -y)).rgb;
    vec3 i = texture(source, TexCoords + vec2(x, -y)).rgb;

    FragColor = (e * 4.0 + (b + d + f + h) * 2.0 + (a + c + g + i)) / 16.0;
}
