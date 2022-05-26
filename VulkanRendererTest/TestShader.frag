#version 450


layout(location = 0) in vec4 in_vertexColor;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(0.5, 0.2, 0.7, 1);   
    out_color = in_vertexColor;
}