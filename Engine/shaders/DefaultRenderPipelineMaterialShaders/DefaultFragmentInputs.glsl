#ifndef DEFAULT_FRAGMENT_INPUTS
#define DEFAULT_FRAGMENT_INPUTS

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS_PER_TILE 1024
#define TILE_SIZE 16

#define MAX_SHADOW_MAP_COUNT 8

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_SPOT 2

#include "PBRMaterial.glsl"
#include "Light.glsl"


layout(location = 0) in vec2 in_vertexUV;
layout(location = 1) in vec3 in_vertexWorldPos;
layout(location = 2) in vec3 in_vertexWorldNormal;
layout(location = 3) in vec4 in_vertexViewPos;
layout(location = 4) in vec4 in_vertexPos;

layout(location = 6) in flat uint in_meshIndex;


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
    uint shadowsEnabled;
    uint cascadeCount;
    uint smoothShadows;
    uint showDebugCascades;
    vec4 cascadeSplits[2];
} shadowPrefs;

layout(set = 0, binding = 8) uniform sampler2DArrayShadow shadowTextures[MAX_SHADOW_MAP_COUNT];
layout(set = 0, binding = 9) uniform samplerCubeShadow shadowCubeTextures[MAX_SHADOW_MAP_COUNT];

layout(set = 0, binding = 10) uniform samplerCube skybox;

layout(set = 0, binding = 32) uniform texture2D textures[];


layout(push_constant) uniform PushConstants {
    mat4 viewMat;
    mat4 projMat;
    vec4 viewPos;
    uint tileCountX;
} pc;


float InShadowCascaded(vec3 worldPos, Light light);
float InShadowCube(vec3 worldPos, Light light);
float InShadow(vec3 worldPos, Light light, uint layer);


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


float InShadowCascaded(vec3 worldPos, Light light) {
    uint cascadeIndex = 0;
	for(uint i = 0; i < shadowPrefs.cascadeCount - 1; ++i) {
		if(in_vertexViewPos.z < shadowPrefs.cascadeSplits[i / 4][i % 4]) {
			cascadeIndex = i + 1;
		}
	}
    return InShadow(worldPos, light, cascadeIndex);
}

float InShadowCube(vec3 worldPos, Light light) {
    if(light.emitShadows == 0) {
        return 0.0;
    }
    ShadowMapData shadowData = shadowMapDataBuffer.shadowMaps[light.shadowMapDataIndex];
    
    vec3 lightDir = worldPos - vec3(light.position);
     
    float far = light.position.w;
    
    float currentDepth = length(lightDir);
    lightDir.x *= -1.0;
    currentDepth /= far;
    
    if(shadowPrefs.smoothShadows == 0) {
        return texture(shadowCubeTextures[shadowData.mapIndex], vec4(lightDir, currentDepth)).r;
    }
    
    const vec3 sampleOffsetDirections[20] = vec3[]
    (
        vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
        vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
        vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
        vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
        vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );   

    float shadow = 0.0;
    int samples = 20;
    float diskRadius = 0.05; 

    for(int i = 0; i < samples; ++i) {
        vec4 uv = vec4(lightDir + sampleOffsetDirections[i] * diskRadius, currentDepth);
        shadow += texture(shadowCubeTextures[shadowData.mapIndex], uv).r;
    }

    return shadow / float(samples);
}

float InShadow(vec3 worldPos, Light light, uint layer) {
    if(light.emitShadows == 0) {
        return 0.0;
    }
    ShadowMapData shadowData = shadowMapDataBuffer.shadowMaps[light.shadowMapDataIndex];


    const mat4 biasMat = mat4( 
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0 );


    vec4 lightSpacePos = biasMat * shadowData.lightMat[layer] * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    vec4 texCoord;
    texCoord.xyw = projCoords.xyz;
    texCoord.z = layer;
    if(shadowPrefs.smoothShadows == 0) {
        return texture(shadowTextures[shadowData.mapIndex], texCoord).r;
    }
    
    vec3 texSize = textureSize(shadowTextures[shadowData.mapIndex], 0);
    vec2 texelSize = 1.0 / texSize.xy;

    const float gaussianKernel[3][3] = {
        { 0.0625, 0.125,  0.0625 },
        { 0.125,  0.25,   0.125 },
        { 0.0625, 0.125,  0.0625 }
    };

    float shadow = 0.0;
    for(int u = -1; u <= 1; u++) {
        for(int v = -1; v <= 1; v++) {
            vec4 uv = vec4(projCoords.xy + vec2(u, v) * texelSize, layer, projCoords.z);
            float gauss = gaussianKernel[u + 1][v + 1];
            shadow += texture(shadowTextures[shadowData.mapIndex], uv).r * gauss;
        }
    }
    return shadow;
}

#endif