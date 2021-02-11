#version 450

layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;
layout(set = 0, binding = 2) uniform sampler2D myTexture;

void main() {
    out_color = vec4(1, 0, 0, 1);
    out_color *= texture(myTexture, in_vertexUV);
    
}