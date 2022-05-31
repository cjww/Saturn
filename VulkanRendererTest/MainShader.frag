#version 450

layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;


layout(set = 0, binding = 0) uniform sampler2D colorTexture;
layout(set = 0, binding = 1) uniform sampler2D blurredColorTexture;


void main() {

    vec2 uv = in_vertexUV;
    uv.y = 1 - uv.y;
    out_color = texture(colorTexture, uv);
    out_color += texture(blurredColorTexture, uv);
    out_color.a = 1.0;
}