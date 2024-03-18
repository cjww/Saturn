#ifndef PBR_FUNCTIONS
#define PBR_FUNCTIONS

#define PI 3.14159265359

#include "DefaultFragmentInputs.glsl"
#include "DefaultRendering.glsl"

vec4 CalculatePBRColor(Material material);
vec4 GetPBRColor(vec3 albedo, vec3 normal, vec3 emission, float metallic, float smoothness, float occlusion, float alpha, float alphaClipThreshold);
vec3 GetPointLightColor(Light light, vec3 normal);
vec3 GetDirectionalLightColor(Light light, vec3 normal);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 FresnelSchlick(vec3 V, vec3 H, vec3 F0);

float InShadow(vec3 worldPos, Light light);

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

vec4 CalculatePBRColor(Material material) {

    vec3 albedoColor = material.albedoColor.rgb;
    if(material.albedoMapCount > 0) {
        vec3 albedoColorTex = texture(sampler2D(textures[material.albedoMapFirst], samp), in_vertexUV).rgb;
        
        //TODO send value to shader
        float gamma = 2.2;
        albedoColorTex = InvertGammaCorrect(albedoColorTex, gamma);
        albedoColor *= albedoColorTex;
    }

    float metallic = material.metallic;
    if(material.metalnessMapCount > 0) {
        metallic = texture(sampler2D(textures[material.metalnessMapFirst], samp), in_vertexUV).r;
    }

    float roughness = material.roughness;
    if(material.roughnessMapCount > 0) {
        roughness = texture(sampler2D(textures[material.roughnessMapFirst], samp), in_vertexUV).g;
    }

    vec3 emissive = material.emissiveColor.rgb * material.emissiveStrength;
    if(material.emissiveMapCount > 0) {
        emissive *= texture(sampler2D(textures[material.emissiveMapFirst], samp), in_vertexUV).rgb;
    }

    float occlusion = 1.0;
    if(material.lightMapCount > 0) {
        occlusion = texture(sampler2D(textures[material.lightMapFirst], samp), in_vertexUV).r;
    }

    return GetPBRColor(albedoColor, in_vertexWorldNormal, emissive, metallic, roughness, occlusion, material.opacity, 0.5);
}

vec4 GetPBRColor(vec3 albedo, vec3 normal, vec3 emission, float metallic, float roughness, float occlusion, float alpha, float alphaClipThreshold) {
    vec3 viewDir = normalize(in_viewPos - in_vertexWorldPos);
    normal = normalize(normal);

    vec3 Lo = vec3(0);
    
    ivec2 pos = ivec2(gl_FragCoord.xy);
    ivec2 tileID = pos / ivec2(TILE_SIZE, TILE_SIZE);
    uint index = tileID.y * pc.tileCountX + tileID.x;
    uint offset = index * MAX_LIGHTS_PER_TILE;
    for(int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices.data[i + offset] != -1; i++) {
        Light light = lightBuffer.lights[lightIndices.data[i + offset]];

        //TODO send value to shader
        int shininess = 32;
        
        vec3 radiance = vec3(0);
        float lightRadius = max(light.position.w, 0.0001);
        float lightIntensity = light.color.a;

        vec3 lightDir = vec3(0);
        vec3 halfVector = vec3(0);

        switch(light.type) {
        case LIGHT_TYPE_POINT: {

            lightDir = normalize(light.position.xyz - in_vertexWorldPos);
            halfVector = normalize(viewDir + lightDir);
            

            float distance = length(light.position.xyz - in_vertexWorldPos);
            
            // Real world attenuation = 1.0 / (distance * distance)
            float fallOff = 1.0;
            float attenuation = max(1.0 - distance / lightRadius, 0.0) / fallOff;
            /*
            // (clamp10(1-(d/lightRadius)^4)^2)/d*d+1
            float attenuation = clamp(1.0 - pow(distance / lightRadius, 4), 1.0, 0.0);
            attenuation = attenuation * attenuation;
            attenuation = attenuation / distance * distance + 1;
            */
            
            radiance = light.color.rgb * attenuation * lightIntensity;

            break;
        }
        case LIGHT_TYPE_DIRECTIONAL: {

            lightDir = normalize(-light.direction.xyz);

            halfVector = normalize(viewDir + lightDir);

            radiance = light.color.rgb * lightIntensity;

            
            radiance *= 1 - InShadow(in_vertexWorldPos, light);

            break;
        }
        case LIGHT_TYPE_SPOT: {
            lightDir = normalize(light.position.xyz - in_vertexWorldPos);
            halfVector = normalize(viewDir + lightDir);
            
            vec3 spotDir = normalize(-light.direction.xyz);
            float theta = dot(lightDir, spotDir);
            
            float cutoff = cos(light.direction.w);
            
            float innerCutoff = cutoff + 0.09;
            float epsilon = innerCutoff - cutoff;
            //float spot = theta > cutoff ? max((theta - cutoff) / epsilon, 0.0) : 0.0;
            float spot = theta > cutoff ? smoothstep(0.0, 1.0, (theta - cutoff) / epsilon) : 0.0;

            float distance = length(light.position.xyz - in_vertexWorldPos);
            
            // Real world attenuation = 1.0 / (distance * distance)
            float fallOff = 1.0;
            float attenuation = max(1.0 - distance / lightRadius, 0.0) / fallOff;
            attenuation *= spot;
            radiance = light.color.rgb * attenuation * lightIntensity;

            break;
        }
        default:
            break;
        }

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, pow(albedo, vec3(2.2)), metallic);

        float NDF = DistributionGGX(normal, halfVector, roughness);
        float G = GeometrySmith(normal, viewDir, lightDir, roughness);
        vec3 F = FresnelSchlick(viewDir, halfVector, F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        float NdotL = max(dot(normal, lightDir), 0.0);

        // Cook-Torrance BRDF
        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * NdotL;
        vec3 specular = numerator / max(denominator, 0.0001);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
        
    }

    vec3 ambient = vec3(0.01) * albedo * occlusion;
    vec3 color = ambient + emission + Lo;
    return vec4(color, alpha);
}

vec3 GetPointLightColor(Light light, vec3 normal) {
    float lightDistance = length(in_vertexWorldPos - light.position.xyz);

    float lightRadius = light.position.w;
    float lightIntensity = light.color.a;
    
    //float attenuation = (1 - lightDistance / lightRadius) / falloff;
    float denom = max(lightDistance - lightRadius, 0.0) / lightRadius + 0.75;
    float attenuation = 1.0 / (denom * denom);
    attenuation = clamp(attenuation, 0.0, 1.0);
    
    vec3 radiance = light.color.rgb * attenuation * lightIntensity; 
    return radiance;
    
}

vec3 GetDirectionalLightColor(Light light, vec3 normal) {
    float lightIntensity = light.color.a;
    vec3 radiance = light.color.rgb * lightIntensity;
    return radiance;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, 0.00001);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / max(denom, 0.00001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(vec3 V, vec3 H, vec3 F0) {
    float cosTheta = max(dot(H, V), 0.0);
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  

float InShadow(vec3 worldPos, Light light) {
            
    ShadowMapData shadowData = shadowMapDataBuffer.shadowMaps[light.shadowMapDataIndex];
    vec4 lightSpacePos = biasMat * shadowData.lightMat * vec4(worldPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    vec2 texSize = textureSize(shadowTextures[shadowData.mapIndex], 0);
    vec2 texelSize = 1.0 / texSize;
    float currentDepth = projCoords.z;
    const float bias = 0.00001;
    
    if(currentDepth > 1.0) {
        return 0.0;
    }

    float shadow = 0.0;
    for(int u = -1; u <= 1; u++) {
        for(int v = -1; v <= 1; v++) {
            vec3 uv = vec3(projCoords.xy + vec2(u, v) * texelSize, 0.0);
            float closestDepth = texture(shadowTextures[shadowData.mapIndex], uv.xy).r;
            if(currentDepth - bias > closestDepth) {
                shadow += 1.0;
            }
        }
    }

    return shadow /= 9;
}

#endif