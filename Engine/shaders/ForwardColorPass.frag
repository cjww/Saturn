#version 450

#define MAX_TEXTURE_MAP_COUNT 8

layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_brightness;

layout(set = 1, binding = 0, std140) uniform MaterialValues {
    vec4 diffuseColor;
    vec4 specularColor;
    
    uint diffuseMapCount;
    uint normalMapCount;
    uint specularMapCount;

    float roughness;
} material;

layout(set = 1, binding = 1) uniform sampler samp;
layout(set = 1, binding = 2) uniform texture2D diffuseMaps[MAX_TEXTURE_MAP_COUNT];
layout(set = 1, binding = 3) uniform texture2D normalMaps[MAX_TEXTURE_MAP_COUNT];
layout(set = 1, binding = 4) uniform texture2D specularMaps[MAX_TEXTURE_MAP_COUNT];




struct Light {
    vec3 color;
    float strength;
    int type;
};

void main() {
    out_brightness = vec4(1, 0, 0, 1);
    

    vec4 diffuseColor = material.diffuseColor;
    int i = 0;
    diffuseColor *= texture(sampler2D(diffuseMaps[i], samp), in_vertexUV);
    
    /*
    if (material.hasDiffuseMap) {
    }
    */

    out_color = vec4(min(diffuseColor.xyz, 1), 1);
}