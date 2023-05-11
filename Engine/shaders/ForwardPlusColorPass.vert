#version 460
#extension GL_GOOGLE_include_directive : enable

#include "DefaultVertexInputs.glsl"

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out flat vec3 out_viewPos;
layout(location = 4) out flat uint out_meshIndex;


void main() {

    Object object = GetObject();

    mat4 worldMat = object.worldMat;
    
    gl_Position = camera.projView * worldMat * in_vertexPosition;

    out_vertexUV = in_vertexUV;
    out_vertexWorldPos = (worldMat * in_vertexPosition).xyz;
    out_vertexWorldNormal = normalize((worldMat * in_vertexNormal).xyz);
    
    out_meshIndex = gl_DrawID;
    out_viewPos = camera.viewPos;

}