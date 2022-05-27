#version 450


layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput colorTexture;

layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput positionTexture;

void main() {
    out_color = subpassLoad(colorTexture);
    out_color += subpassLoad(positionTexture);
    
    //out_color = min(out_color, vec4(1, 1, 1, 1));
}