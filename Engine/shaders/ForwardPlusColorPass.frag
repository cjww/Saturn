#version 460
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS_PER_TILE 1024
#define TILE_SIZE 16

layout(location = 0) in vec2 in_vertexUV;
layout(location = 1) in vec3 in_vertexWorldPos;
layout(location = 2) in vec3 in_vertexWorldNormal;
layout(location = 3) in flat vec3 in_viewPos;
layout(location = 4) in flat uint in_meshIndex;

layout(location = 0) out vec4 out_color;

struct Material {
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
};

struct Light {
    vec4 color;
    vec3 position;
    float intensity;
    float attenuationRadius;
    uint type;
};

layout(set = 0, binding = 1, std140) readonly buffer Lights {
    uint lightCount;
    Light lights[];
} lightBuffer;


layout(set = 0, binding = 2, std140) readonly buffer Materials {
    Material materials[];
} materialBuffer;

layout(set = 0, binding = 3) readonly buffer MaterialIndices {
	uint data[];
} materialIndices;


layout(set = 0, binding = 4) readonly buffer LightIndices {
	uint data[];
} lightIndices;

layout(set = 0, binding = 5) uniform sampler samp;
layout(set = 0, binding = 6) uniform texture2D textures[];

layout(push_constant) uniform PushConstants {
    mat4 projView;
    vec3 viewPos;
    uint tileCountX;
} pc;

void main() {

    ivec2 pos = ivec2(gl_FragCoord.xy);
    ivec2 tileID = pos / ivec2(TILE_SIZE, TILE_SIZE);
    uint index = tileID.y * pc.tileCountX + tileID.x;

    uint materialIndex = materialIndices.data[in_meshIndex];

    Material material = materialBuffer.materials[materialIndex];

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
        uint offset = index * MAX_LIGHTS_PER_TILE;
        
        for(int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices.data[i + offset] != -1; i++) {
            Light light = lightBuffer.lights[lightIndices.data[i + offset]];

            vec3 toLight = light.position - in_vertexWorldPos;
            float lightDistance = length(toLight);
            toLight = normalize(toLight);
            
            float diffuseFactor = max(dot(in_vertexWorldNormal, toLight), 0.0);
            //float attenuation = light.attenuationRadius / lightDistance;
            float attenuation = (1 - lightDistance / (light.attenuationRadius)) / 0.2;
            attenuation = clamp(attenuation, 0.0, 1.0);
            vec4 radiance = light.color * attenuation * light.intensity; 

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
        
    out_color = vec4(finalColor.xyz, material.opacity);

    //float brightness = (0.2126 * finalColor.r + 0.7152 * finalColor.g + 0.0722 * finalColor.b);

}