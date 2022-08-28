#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#define MAX_TEXTURE_MAP_COUNT 4

layout(location = 0) in vec2 in_vertexUV;
layout(location = 1) in vec3 in_vertexWorldPos;
layout(location = 2) in vec3 in_vertexWorldNormal;
layout(location = 3) in vec3 in_viewPos;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_brightness;

layout(set = 1, binding = 0, std140) uniform MaterialValues {
    vec4 diffuseColor;
    vec4 specularColor;
    vec4 ambientColor;
    vec4 emissiveColor;
    
    uint diffuseMapFirst;
    uint diffuseMapCount;
    uint normalMapFirst;
    uint normalMapCount;
    uint specularMapFirst;
    uint specularMapCount;
    uint emissiveMapFirst;
    uint emissiveMapCount;
    uint lightMapFirst;
    uint lightMapCount;
    
    float opacity;
    float shininess;
    float metallic;
} material;

layout(set = 1, binding = 1) uniform sampler samp;
layout(set = 1, binding = 2) uniform texture2D textures[];

struct Light {
    vec4 color;
    vec3 position;
    float strength;
    uint type;
};

layout(set = 0, binding = 1, std140) readonly buffer Lights {
    uint lightCount;
    Light lights[4096];
} lightBuffer;

void main() {
    /*
    out_color = vec4(material.opacity, 0, 0, 1);
    return;
    out_color = material.diffuseColor * texture(sampler2D(textures[material.diffuseMapFirst], samp), in_vertexUV);
    return;
    */

    vec4 ambientColor = material.ambientColor * 0.1;
    vec4 diffuseColor = vec4(0, 0, 0, 1);
    vec4 specularColor = vec4(0, 0, 0, 1);

    vec4 objectColor = material.diffuseColor;
    if(material.diffuseMapCount > 0)
        objectColor *= texture(sampler2D(textures[material.diffuseMapFirst], samp), in_vertexUV);
    
    if(material.specularMapCount > 0) {
        specularColor += texture(sampler2D(textures[material.specularMapFirst], samp), in_vertexUV);
    }

    if(material.emissiveMapCount > 0) {
        objectColor = material.emissiveColor;
        diffuseColor = vec4(1, 1, 1, 1);
        objectColor *= texture(sampler2D(textures[material.emissiveMapFirst], samp), in_vertexUV);
    }
    else {
        vec3 viewDir = normalize(in_viewPos - in_vertexWorldPos);
        for(int i = 0; i < lightBuffer.lightCount; i++) {
            Light light = lightBuffer.lights[i];

            vec3 toLight = light.position - in_vertexWorldPos;
            float lightDistance = length(toLight);
            toLight = normalize(toLight);


            float diffuseFactor = max(dot(in_vertexWorldNormal, toLight), 0.0);
            float attenuation = 1 / (lightDistance * lightDistance);
            vec4 radiance = light.color * attenuation * light.strength; 

            diffuseColor += radiance * diffuseFactor;
            
            vec3 reflectDir = reflect(-toLight, in_vertexWorldNormal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            specularColor += radiance * spec;
        }
    }
    
    float occlusion = 1.0;
    if(material.lightMapCount > 0) {
        vec4 color = texture(sampler2D(textures[material.lightMapFirst], samp), in_vertexUV);
        occlusion = color.r;
    }
  
    vec4 finalColor = (ambientColor + diffuseColor + specularColor) * occlusion * objectColor;
        
    out_color = vec4(min(finalColor.xyz, 1), material.opacity);

    float brightness = (0.2126 * finalColor.r + 0.7152 * finalColor.g + 0.0722 * finalColor.b);
    out_brightness = vec4(0, 0, 0, 1);
    if(brightness > 0.8) {
        out_brightness = vec4(out_color.xyz, floor(out_color.a));
    }
}