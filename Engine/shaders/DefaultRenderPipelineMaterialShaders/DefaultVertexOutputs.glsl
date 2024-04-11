#ifndef DEFAULT_VERTEX_OUTPUTS
#define DEFAULT_VERTEX_OUTPUTS

#include "DefaultVertexInputs.glsl"

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out vec4 out_vertexViewPos;
layout(location = 4) out vec4 out_vertexPos;

layout(location = 6) out flat uint out_meshIndex;

void DefaultPassThrough() {
    Object object = GetObject();

    out_vertexWorldPos = (object.modelMatrix * in_vertexPosition).xyz;
    out_vertexViewPos = camera.viewMat * vec4(out_vertexWorldPos, 1.0);
    gl_Position = camera.projMat * out_vertexViewPos;

    out_vertexUV = in_vertexUV;
    out_vertexWorldNormal = normalize((object.modelMatrix * in_vertexNormal).xyz);
    out_vertexPos = in_vertexPosition;

    out_meshIndex = gl_DrawID;
}

#endif