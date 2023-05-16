#ifndef DEFAULT_VERTEX_OUTPUTS
#define DEFAULT_VERTEX_OUTPUTS

#include "DefaultVertexInputs.glsl"

layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out vec3 out_vertexWorldPos;
layout(location = 2) out vec3 out_vertexWorldNormal;
layout(location = 3) out flat vec3 out_viewPos;
layout(location = 4) out flat uint out_meshIndex;

void DefaultPassThrough() {
    Object object = GetObject();

    gl_Position = camera.projView * object.modelMatrix * in_vertexPosition;

    out_vertexUV = in_vertexUV;
    out_vertexWorldPos = (object.modelMatrix * in_vertexPosition).xyz;
    out_vertexWorldNormal = normalize((object.modelMatrix * in_vertexNormal).xyz);
    
    out_meshIndex = gl_DrawID;
    out_viewPos = camera.viewPos;
}

#endif