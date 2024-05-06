#ifndef PBRMATERIAL_GLSL
#define PBRMATERIAL_GLSL

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

#endif