#version 450

layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_color1;

//layout(set = 0, binding = 2) uniform sampler2D myTexture;




struct Light {
    vec3 color;
    float strength;
    int type;
};
/*
layout(set = 1, binding = 1) uniform lights {
    Light lights[64];
};
*/

void main() {
    out_color = vec4(1, 1, 1, 1);
    out_color1 = vec4(1, 0, 0, 1);
    //out_color += texture(myTexture, in_vertexUV);
    //out_color = min(out_color, 1);

}