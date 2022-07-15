#version 450

vec4 skyboxVertices[36] = vec4[](
    // positions          
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4(-1.0, -1.0, -1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4( 1.0,  1.0, -1.0, 1.0),
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4(-1.0, -1.0, -1.0, 1.0),
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4(-1.0,  1.0,  1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4( 1.0, -1.0,  1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4( 1.0,  1.0, -1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4(-1.0,  1.0,  1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4( 1.0, -1.0,  1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4( 1.0,  1.0, -1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4( 1.0,  1.0,  1.0, 1.0),
    vec4(-1.0,  1.0,  1.0, 1.0),
    vec4(-1.0,  1.0, -1.0, 1.0),
    vec4(-1.0, -1.0, -1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4( 1.0, -1.0, -1.0, 1.0),
    vec4(-1.0, -1.0,  1.0, 1.0),
    vec4( 1.0, -1.0,  1.0, 1.0)
);

layout(location = 0) out vec3 out_texCoord;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 projection;
}ubo;

void main() {
    mat4 view = ubo.view;
    view[3] = vec4(0, 0, 0, 1);
    gl_Position = ubo.projection * view * skyboxVertices[gl_VertexIndex];
    out_texCoord = skyboxVertices[gl_VertexIndex].xyz * vec3(-1, 1, 1);
}