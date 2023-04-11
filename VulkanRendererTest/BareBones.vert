#version 450

layout(location = 0) in vec4 in_vertexPosition;

layout(set = 0, binding = 0) uniform Scene {
    mat4 view;
    mat4 projection;
} scene;

layout(push_constant) uniform Pc {
    mat4 model;
}pc;


void main() {

    gl_Position = scene.projection * scene.view * pc.model * in_vertexPosition;

}