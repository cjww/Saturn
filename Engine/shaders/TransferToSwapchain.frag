#version 450

layout(set = 0, binding = 0) uniform sampler2D colorTexture;

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 in_vertPos;


void main() {
	vec2 uv = in_vertPos * 0.5 + 0.5;
	vec3 color = texture(colorTexture, uv).rgb;
	out_color = vec4(min(color, 1), 1);
}