#version 460

layout(location = 1) in vec3 in_vertexWorldPos;

layout(push_constant) uniform PushConstants {
    mat4 viewMat;
    mat4 projMat;
    vec4 viewPos;
    uint linearizeDepth;
}pc;

void main() {
    if(pc.linearizeDepth == 0) {
        gl_FragDepth = gl_FragCoord.z;
        return;
    }

    float lightDistance = length(in_vertexWorldPos - pc.viewPos.xyz);
    float far = pc.viewPos.w;
    lightDistance /= far;

    gl_FragDepth = lightDistance;
}
