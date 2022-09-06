#version 460
#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexNormal;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out flat vec3 out_viewPos;
layout(location = 4) out flat uint out_meshIndex;

struct Object {
    mat4 worldMat;
};

layout(set = 0, binding = 0) readonly buffer Objects {
    Object objects[2048];
} objectBuffer;

layout(push_constant) uniform Camera {
    mat4 projView;
    vec3 viewPos;
} camera;

void main() {

    Object object = objectBuffer.objects[gl_InstanceIndex];

    mat4 worldMat = object.worldMat;
    
    gl_Position = camera.projView * worldMat * in_vertexPosition;

    out_vertexUV = in_vertexUV;
    out_vertexWorldPos = (worldMat * in_vertexPosition).xyz;
    out_vertexWorldNormal = normalize((worldMat * in_vertexNormal).xyz);
    
    out_meshIndex = gl_DrawID;
    out_viewPos = camera.viewPos;

}