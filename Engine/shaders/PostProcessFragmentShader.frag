#version 450
//#extension GL_KHR_vulkan_glsl : enable


layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;

layout(location = 0) out vec4 out_color;

void main() {
    
    out_color = subpassLoad(inputColor);
}