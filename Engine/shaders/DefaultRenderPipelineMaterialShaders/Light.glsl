#ifndef LIGHT_GLSL
#define LIGHT_GLSL

struct Light {
    vec4 color;     // vec3 color, float intensity
    vec4 position;  // vec3 position, float attenuationRadius
    vec4 direction; // vec3 direction
    uint type;
    uint emitShadows;
    uint shadowMapDataIndex;
};

#endif