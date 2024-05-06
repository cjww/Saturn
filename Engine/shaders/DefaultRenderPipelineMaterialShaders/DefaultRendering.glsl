#ifndef DEFAULT_RENDERING_GLSL
#define DEFAULT_RENDERING_GLSL

vec3 InvertGammaCorrect(vec3 color, float gamma);


vec3 InvertGammaCorrect(vec3 color, float gamma) {
    return pow(color, vec3(gamma));
}



#endif