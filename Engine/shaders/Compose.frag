#version 450

layout(set = 0, binding = 0) uniform sampler2D colorTexture;
layout(set = 0, binding = 1) uniform sampler2D lightmap;

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 in_vertPos;

void main() {
	vec2 uv = in_vertPos / 2 + 0.5;
	vec4 color = texture(colorTexture, uv);
	color += texture(lightmap, uv);
	
	out_color = vec4(min(color, 1).xyz, 1);
}