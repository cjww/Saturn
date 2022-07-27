#version 430

layout(location = 0) in vec4 in_vertexPosition;

void main() {
    gl_Position = in_vertexPosition;
}