#version 460
#extension GL_GOOGLE_include_directive : enable

#include "DefaultRenderPipelineMaterialShaders/DefaultVertexInputs.glsl"

void main() {
    Object object = GetObject();
    gl_Position = camera.projView * object.modelMatrix * in_vertexPosition;
}