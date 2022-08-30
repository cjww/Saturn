#version 450


layout(location = 0) in vec4 in_vertexColor;
layout(location = 1) in vec2 in_vertexUV;
layout(location = 2) in vec4 in_vertexWorldPos;


layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform sampler samp;
layout(set = 0, binding = 2) uniform texture2D textures[2];

void main() {
    out_color = texture(sampler2D(textures[0], samp), in_vertexUV);

}