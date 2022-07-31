#version 450

layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_brightness;

layout(set = 2, binding = 0) uniform MaterialValues {
    vec4 diffuseColor;
    vec4 specularColor;
    float roughness;
} material;

layout(set = 2, binding = 1) uniform sampler samp;
layout(set = 2, binding = 2) uniform texture2D diffuseMap;
layout(set = 2, binding = 3) uniform texture2D normalMap;
layout(set = 2, binding = 4) uniform texture2D specularMap;




struct Light {
    vec3 color;
    float strength;
    int type;
};

void main() {
    out_brightness = vec4(1, 0, 0, 1);
    out_color = texture(sampler2D(diffuseMap, samp), in_vertexUV);
}