#version 460
#extension GL_GOOGLE_include_directive : enable

#include "DefaultRenderPipelineMaterialShaders/DefaultFragmentInputs.glsl"
#include "DefaultRenderPipelineMaterialShaders/PBRFunctions.glsl"

layout(location = 0) out vec4 out_color;

void main() {
    Material material = GetMaterial();
    out_color = CalculatePBRColor(material);
}
