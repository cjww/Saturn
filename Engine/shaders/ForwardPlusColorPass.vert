#version 450
layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexNormal;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out vec3 out_viewPos;

layout(push_constant) uniform PushConstants {
    mat4 world[4];
} pc;

layout(set = 0, binding = 0) uniform Scene {
    mat4 projView;
    vec3 viewPos;
} sceneUbo;

void main() {

    gl_Position = sceneUbo.projView * pc.world[gl_InstanceIndex] * in_vertexPosition;
    
    out_vertexUV = in_vertexUV;
    out_vertexWorldPos = (pc.world[gl_InstanceIndex] * in_vertexPosition).xyz;
    out_vertexWorldNormal = normalize((pc.world[gl_InstanceIndex] * in_vertexNormal).xyz);
    out_viewPos = sceneUbo.viewPos;
}