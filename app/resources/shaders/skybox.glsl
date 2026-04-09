//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}

//#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

uniform bool fogEnabled;
uniform float fogIntensity;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;

void main()
{
    vec3 color = texture(skybox, TexCoords).rgb;

    if (fogEnabled) {
        vec3 dir = normalize(TexCoords);
        float heightFactor = abs(dir.y);
        float fogFactor = exp(-pow((1.0 - heightFactor) * fogIntensity, 2.0));
        color = mix(fogColor, color, fogFactor);
    }

    FragColor = vec4(color, 1.0);
}