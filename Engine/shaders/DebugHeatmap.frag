#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS_PER_TILE 1024

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) readonly buffer LightIndices {
	uint data[];
} lightIndices;

layout(push_constant) uniform PushConstants {
    uint tileCountX;
} pc;

void main() {
    ivec2 pos = ivec2(gl_FragCoord.xy);
    uint index = pos.y * pc.tileCountX + pos.x;

    uint offset = index * MAX_LIGHTS_PER_TILE;
    uint lightCount = 0;
    for(int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices.data[i + offset] != -1; i++) {
        lightCount++;
    }
    out_color = vec4(0, 0, 0, 0);
    
    uint colorLimits[] = {
        0,
        2,
        5,
        10
    };

    vec4 colors[] = {
        vec4(0.0, 0.0, 0.0, 0.7),
        vec4(0.0, 0.0, 1.0, 0.7),
        vec4(0.0, 1.0, 0.0, 0.7),
        vec4(1.0, 0.0, 0.0, 0.7)
    };
    

    for(int i = 0; i < 3; i++) {
        if(lightCount > colorLimits[i]) {
            out_color = mix(colors[i], colors[i + 1], float(lightCount) / float(colorLimits[i + 1] - colorLimits[i]));
        }
    }
}