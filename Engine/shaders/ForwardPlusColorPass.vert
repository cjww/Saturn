#version 450
layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexNormal;
layout(location = 2) in vec2 in_vertexUV;

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out flat vec3 out_viewPos;

struct Object {
    mat4 worldMat;
};

layout(set = 0, binding = 0) readonly buffer Objects {
    Object objects[2048];
} objectBuffer;

layout(set = 0, binding = 1) uniform Scene {
    mat4 projView;
    vec3 viewPos;
} sceneUbo;

void main() {

    mat4 worldMat = objectBuffer.objects[gl_InstanceIndex].worldMat;
    
    gl_Position = sceneUbo.projView * worldMat * in_vertexPosition;

    out_vertexUV = in_vertexUV;
    out_vertexWorldPos = (worldMat * in_vertexPosition).xyz;
    out_vertexWorldNormal = normalize((worldMat * in_vertexNormal).xyz);
    //out_viewPos = sceneUbo.viewPos;
    out_viewPos.x = gl_InstanceIndex;
}