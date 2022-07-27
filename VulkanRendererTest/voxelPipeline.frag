#version 430

layout(location = 0) out vec4 out_color;

layout(set = 0, binding = 0, rgba8) uniform image2D tex;
layout(set = 0, binding = 1, rgba8) uniform image3D tex3D;

layout(set = 0, binding = 2) uniform sampler3D texSamp;

void main() {
    
    out_color = vec4(1, 0, 1, 1);
    
    imageStore(tex3D, ivec3(0, 0, 0), vec4(1, 1, 0, 1));
    out_color = imageLoad(tex3D, ivec3(0, 0, 0));

    imageStore(tex, ivec2(0, 0), vec4(0, 0, 1, 1));
    out_color = imageLoad(tex, ivec2(0, 0));
    
    out_color = texture(texSamp, vec3(0, 0, 0));

}