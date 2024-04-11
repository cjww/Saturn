#ifndef DEFAULT_FRAGMENT_INPUTS
#define DEFAULT_FRAGMENT_INPUTS

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS_PER_TILE 1024
#define TILE_SIZE 16

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_SPOT 2


layout(location = 0) in vec2 in_vertexUV;
layout(location = 1) in vec3 in_vertexWorldPos;
layout(location = 2) in vec3 in_vertexWorldNormal;
layout(location = 3) in vec4 in_vertexViewPos;
layout(location = 4) in vec4 in_vertexPos;

layout(location = 6) in flat uint in_meshIndex;

struct Material {
    vec4 albedoColor;
    vec3 emissiveColor;
    float emissiveStrength;

    uint albedoMapFirst;
    uint albedoMapCount;
    uint normalMapFirst;
    uint normalMapCount;
    uint metalnessMapFirst;
    uint metalnessMapCount;
    uint roughnessMapFirst;
    uint roughnessMapCount;
    uint emissiveMapFirst;
    uint emissiveMapCount;
    uint lightMapFirst;
    uint lightMapCount;
    
    float opacity;
    float roughness;
    float metallic;
};

const Material defaultMaterial = {
    vec4(1, 1, 1, 1),
    vec3(0, 0, 0),
    1.0,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    0, 0,
    1.0,
    0.5,
    0.0
};

struct Light {
    vec4 color;     // vec3 color, float intensity
    vec4 position;  // vec3 position, float attenuationRadius
    vec4 direction; // vec3 direction
    uint type;
    uint shadowMapDataIndex;
};

struct ShadowMapData {
    mat4 lightMat[6];
    uint mapIndex;
};

layout(set = 0, binding = 1, std140) readonly buffer Lights {
    uint lightCount;
    Light lights[];
} lightBuffer;


layout(set = 0, binding = 2, std140) readonly buffer Materials {
    Material materials[];
} materialBuffer;

layout(set = 0, binding = 3) readonly buffer MaterialIndices {
	int data[];
} materialIndices;


layout(set = 0, binding = 4) readonly buffer LightIndices {
	uint data[];
} lightIndices;

layout(set = 0, binding = 5, std140) readonly buffer ShadowMaps {
    ShadowMapData shadowMaps[];
} shadowMapDataBuffer;

layout(set = 0, binding = 6) uniform sampler samp;

layout(set = 0, binding = 7) uniform ShadowPreferences {
    uint cascadeCount;
    uint smoothShadows;
    uint showDebugCascades;
    vec4 cascadeSplits[2];
} shadowPrefs;

layout(set = 0, binding = 8) uniform sampler2DArrayShadow shadowTextures[8];
layout(set = 0, binding = 9) uniform samplerCubeShadow shadowCubeTextures[8];

layout(set = 0, binding = 32) uniform texture2D textures[];


layout(push_constant) uniform PushConstants {
    mat4 viewMat;
    mat4 projMat;
    vec4 viewPos;
    uint tileCountX;
} pc;

Material _GetMaterial(int index) {
    if (index == -1) {
        return defaultMaterial;
    }
    return materialBuffer.materials[index];
}

Material GetMaterial() {
    int materialIndex = materialIndices.data[in_meshIndex];
    return _GetMaterial(materialIndex);
}

#endif