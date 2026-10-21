//#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;

#include "frame.glsl"

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    // Drop the translation of the view matrix, so the skybox always surrounds the camera.
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}

//#shader fragment
#version 330 core
out vec4 FragColor;

#include "frame.glsl"
#include "fog.glsl"

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    // Underwater, the sky is only visible through the water surface; past the surface everything is murky water.
    if (cameraUnderwater) {
        FragColor = vec4(underwaterScatter(), 1.0);
        return;
    }
    vec3 color = texture(skybox, TexCoords).rgb;

    if (fogEnabled) {
        vec3 dir = normalize(TexCoords);
        float heightFactor = abs(dir.y);
        float fogFactor = exp(-pow((1.0 - heightFactor) * fogIntensity, 2.0));
        color = mix(fogColor, color, fogFactor);
    }

    FragColor = vec4(color, 1.0);
}
