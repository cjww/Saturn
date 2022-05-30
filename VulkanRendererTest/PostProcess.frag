#version 450


layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput colorTexture;

layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput positionTexture;

void main() {
    vec4 color = subpassLoad(colorTexture);
    vec4 position = subpassLoad(positionTexture);

    vec3 lightPos = vec3(0, 0, 10);
    vec3 toLight = lightPos - position.xyz;

    out_color = color / length(toLight);
    
    out_color = normalize(out_color);
}