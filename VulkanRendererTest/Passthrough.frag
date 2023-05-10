#version 450
#extension GL_EXT_nonuniform_qualifier : enable

layout (location = 0) out vec4 out_color;
layout (location = 0) in vec2 in_vertexUV;
layout (location = 1) in vec4 in_vertexColor;


layout(set = 1, binding = 1) uniform sampler2D myTex[];

layout(push_constant) uniform PC {
    layout(offset = 16) vec4 color;
    uint texIndex;
};

void main() {
    
	out_color = texture(myTex[texIndex], in_vertexUV) * in_vertexColor;

}
