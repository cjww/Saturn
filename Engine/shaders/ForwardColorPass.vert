#version 450
layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;

layout(push_constant) uniform PushConstants {
    mat4 world[4];
} pc;

layout(set = 0, binding = 0) uniform Scene {
    mat4 projView;
} sceneUbo;

void main() {

    gl_Position = sceneUbo.projView * pc.world[gl_InstanceIndex] * in_vertexPosition;
    
    out_vertexUV = in_vertexUV;
}