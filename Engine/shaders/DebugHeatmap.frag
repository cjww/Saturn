#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_LIGHTS_PER_TILE 1024

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0) readonly buffer LightIndices {
	uint data[];
} lightIndices;

void main() {
    ivec2 pos = ivec2(gl_FragCoord.xy);
    uint index = pos.y * 88 + pos.x;

    uint offset = index * MAX_LIGHTS_PER_TILE;
    uint lightCount = 0;
    for(int i = 0; i < MAX_LIGHTS_PER_TILE && lightIndices.data[i + offset] != -1; i++) {
        lightCount++;
    }
    out_color = vec4(0, 0, 0, 1);
    if(lightCount > 0 && lightCount < 2) {
        out_color.b = 1;
    }
    else if(lightCount >= 2 && lightCount < 5) {
        out_color.g = 1;
    }
    else if(lightCount >= 5) {
        out_color.r = 1;
    }
}