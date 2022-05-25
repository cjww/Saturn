#version 450

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) uniform MyUniform {
    vec4 color;
} ubo;

void main() {
    out_color = vec4(0.5, 0.2, 0.7, 1);   
}