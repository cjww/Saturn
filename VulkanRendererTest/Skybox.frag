#version 450


layout(location = 0) in vec3 in_texCoord;

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 1) uniform samplerCube cubeMapTexture;

void main() {
    out_color = texture(cubeMapTexture, in_texCoord);
}