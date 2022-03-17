#version 450
//#extension GL_KHR_vulkan_glsl : enable


layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 in_vertPos;


void main() {
    out_color = subpassLoad(inputColor);
	float t = 0.5f;
    vec3 c;
	float l,z=t;
	for(int i=0;i<3;i++) {
		vec2 uv,p=in_vertPos;
		uv=p;
		p-=.5;
		p.x*=1000/600;
		z+=.07;
		l=length(p);
		uv+=p/l*(sin(z)+1.)*abs(sin(l*9.-z-z));
		c[i]=.01/length(mod(uv,1.)-.5);
	}
	out_color += vec4(c/l,t);
}