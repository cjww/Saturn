#ifndef DEFAULT_RENDERING_GLSL
#define DEFAULT_RENDERING_GLSL

#include "DefaultFragmentInputs.glsl"

vec3 InvertGammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(gamma));
}

#endif