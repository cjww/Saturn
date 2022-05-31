#version 450


layout(location = 0) in vec2 in_vertexUV;

layout(location = 0) out vec4 out_color;
layout(location = 1) out vec4 out_brightness;


layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput colorTexture;

layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput positionTexture;

void main() {
    vec4 color = subpassLoad(colorTexture);
    vec4 position = subpassLoad(positionTexture);

    vec3 lightPos = vec3(0, 0, 10);
    vec3 toLight = lightPos - position.xyz;
    float lightFactor = length(toLight) / 1000.0;

    out_color = color / lightFactor;
    out_color = min(out_color, vec4(1));
    float brightness = (0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 * out_color.b);
    out_brightness = vec4(0); 
    if(brightness > 0.9) {
        out_brightness = out_color * brightness;
    }

    //out_color = color;
    //out_color = normalize(out_color);
}