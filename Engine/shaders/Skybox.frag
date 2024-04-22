#version 460

#include "DefaultRenderPipelineMaterialShaders/DefaultRendering.glsl"

layout(location = 0) out vec4 out_color;
layout(location = 0) in vec3 in_textureCoords;

layout(set = 0, binding = 0) uniform samplerCube skybox;

void main() {
    vec3 color = texture(skybox, in_textureCoords).rgb;
    out_color = vec4(InvertGammaCorrect(color, 2.2), 1.0);
}