#version 450
//#extension GL_KHR_vulkan_glsl : enable


//layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout(set = 0, binding = 0) uniform sampler2D texSampler;


layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 in_vertPos;

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

void main() {
    //out_color = subpassLoad(inputColor);
	vec2 in_uv = in_vertPos / 2 + 0.5;
	out_color = texture(texSampler, in_uv);
	return;
	//declare stuff
	const int mSize = 11;
	const int kSize = (mSize-1)/2;
	float kernel[mSize];
	vec3 final_colour = vec3(0.0);
	
	//create the 1-D kernel
	float sigma = 7.0;
	float Z = 0.0;
	for (int j = 0; j <= kSize; ++j)
	{
		kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
	}
	
	//get the normalization factor (as the gaussian has been clamped)
	for (int j = 0; j < mSize; ++j)
	{
		Z += kernel[j];
	}
	
	//read out the texels
	for (int i=-kSize; i <= kSize; ++i)
	{
		for (int j=-kSize; j <= kSize; ++j)
		{
			vec2 inUvSpace = vec2(float(i) / 1200.0,float(j) / 800.0);

			final_colour += kernel[kSize+j]*kernel[kSize+i]*texture(texSampler, (in_uv.xy+inUvSpace)).rgb;

		}
	}
	
	
	out_color = vec4(final_colour/(Z*Z), 1.0);
}