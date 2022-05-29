#version 450


layout(location = 0) in vec4 in_vertexColor;
layout(location = 1) in vec2 in_vertexUV;
layout(location = 2) in vec4 in_vertexWorldPos;

layout(set = 0, binding = 2) uniform sampler2D myTexture;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_position;

void main() {
    out_color = vec4(0.5, 0.2, 0.7, 1);   
    out_color = in_vertexColor;

    out_color = texture(myTexture, in_vertexUV);
    out_position = in_vertexWorldPos;
}