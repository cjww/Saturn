#version 450

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

void main() {
    ivec2 extent = textureSize(colorTexture, 0);
    vec2 uv = gl_FragCoord.xy / extent;
    out_color = texture(colorTexture, uv);
}