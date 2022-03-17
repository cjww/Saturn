#version 450
#extension GL_ARB_shader_viewport_layer_array : enable
#extension GL_NV_viewport_array2 : enable

vec4 quad[6] = {
    vec4(-1, 1, 0, 1),
    vec4(1, 1, 0, 1),
    vec4(-1, -1, 0, 1),
    vec4(1, 1, 0, 1),
    vec4(1, -1, 0, 1),
    vec4(-1, -1, 0, 1)
};

layout(location = 1) out vec2 out_vertPos;

void main() {

    gl_Position = quad[gl_VertexIndex];  
    out_vertPos = gl_Position.xy;    
}