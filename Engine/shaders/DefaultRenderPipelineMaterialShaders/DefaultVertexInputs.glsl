#ifndef DEFAULT_VERTEX_INPUTS
#define DEFAULT_VERTEX_INPUTS

#extension GL_ARB_shader_draw_parameters : enable

layout(location = 0) in vec4 in_vertexPosition;
layout(location = 1) in vec4 in_vertexNormal;
layout(location = 2) in vec2 in_vertexUV;

struct Object {
    mat4 modelMatrix;
};

layout(set = 0, binding = 0) readonly buffer Objects {
    Object objects[];
} objectBuffer;

layout(push_constant) uniform Camera {
    mat4 viewMat;
    mat4 projMat;
    vec3 viewPos;
} camera;

Object GetObject() {
    return objectBuffer.objects[gl_InstanceIndex];
}



#endif