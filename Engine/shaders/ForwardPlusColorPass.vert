#version 460
#extension GL_GOOGLE_include_directive : enable

#include "DefaultRenderPipelineMaterialShaders/DefaultVertexInputs.glsl"
#include "DefaultRenderPipelineMaterialShaders/DefaultVertexOutputs.glsl"

void main() {
    DefaultPassThrough();
}