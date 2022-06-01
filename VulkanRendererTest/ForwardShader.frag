#version 450


layout(location = 0) in vec4 in_vertexColor;
layout(location = 1) in vec2 in_vertexUV;
layout(location = 2) in vec4 in_vertexWorldPos;

layout(set = 0, binding = 1) uniform sampler2D myTexture;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(myTexture, in_vertexUV);
}