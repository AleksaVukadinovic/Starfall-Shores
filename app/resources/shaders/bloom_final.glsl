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

    FragColor = vec4(result, 1.0);
}