#ifndef PBR_FUNCTIONS
#define PBR_FUNCTIONS

#include "DefaultFragmentInputs.glsl"
#include "DefaultRendering.glsl"

vec4 GetPBRColor(vec3 albedo, vec3 normal, vec3 emission, float metallic, float smoothness, float occlusion, float alpha, float alphaClipThreshold) {
    vec3 albedoColor = albedo;
    albedoColor *= 0.01;

    ivec2 pos = ivec2(gl_FragCoord.xy);
    ivec2 tileID = pos / ivec2(TILE_SIZE, TILE_SIZE);
    uint index = tileID.y * pc.tileCountX + tileID.x;

    vec3 viewDir = normalize(in_viewPos - in_vertexWorldPos);
    uint offset = index * MAX_LIGHTS_PER_TILE;
    
    vec3 specularColor = vec3(0, 0, 0);
    for(int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices.data[i + offset] != -1; i++) {
        Light light = lightBuffer.lights[lightIndices.data[i + offset]];

        //TODO send value to shader
        int shininess = 32;
            
        switch(light.type) {
        case LIGHT_TYPE_POINT: {

            vec3 toLight = light.position.xyz - in_vertexWorldPos;
            float lightDistance = length(toLight);
            toLight = normalize(toLight);
            
            float diffuseFactor = max(dot(normal, toLight), 0.0);
            //TODO send value to shader
            float falloff = 1.0;
            float attenuation = (1 - lightDistance / (light.position.w)) / falloff;
            attenuation = clamp(attenuation, 0.0, 1.0);
            vec3 radiance = light.color.rgb * attenuation * light.color.a; 

            albedoColor += radiance * diffuseFactor;
            
            vec3 reflectDir = reflect(-toLight, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            specularColor += radiance * spec;
            break;
        }
        case LIGHT_TYPE_DIRECTIONAL: {

            float diffuseFactor = max(dot(normal, -light.direction.xyz), 0.0);
            vec3 radiance = light.color.rgb * light.color.a;
            albedoColor += radiance * diffuseFactor;

            vec3 reflectDir = reflect(light.direction.xyz, normal);
            
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
            specularColor += radiance * spec;
            break;
        }
        default:
            break;
        }
    }
    emission *= 2.0;
    return vec4((albedoColor + specularColor) * occlusion * albedo + emission, alpha);
}


vec4 CalculatePBRColor(Material material) {

    vec3 albedoColor = material.albedoColor.rgb;
    if(material.albedoMapCount > 0) {
        vec3 albedoColorTex = texture(sampler2D(textures[material.albedoMapFirst], samp), in_vertexUV).rgb;
        
        //TODO send value to shader
        float gamma = 2.2;
        albedoColorTex = InvertGammaCorrect(albedoColorTex, gamma);
        albedoColor *= albedoColorTex;
    }

    float metallic = 0.0;
    if(material.metalnessMapCount > 0) {
        metallic = texture(sampler2D(textures[material.metalnessMapFirst], samp), in_vertexUV).r;
    }

    vec3 emissive = material.emissiveColor.rgb;
    if(material.emissiveMapCount > 0) {
        emissive += texture(sampler2D(textures[material.emissiveMapFirst], samp), in_vertexUV).rgb;
    }

    float occlusion = 1.0;
    if(material.lightMapCount > 0) {
        occlusion = texture(sampler2D(textures[material.lightMapFirst], samp), in_vertexUV).r;
    }
    
    return GetPBRColor(albedoColor, in_vertexWorldNormal, emissive, metallic, 0.0, occlusion, material.opacity, 0.5);
}

#endif