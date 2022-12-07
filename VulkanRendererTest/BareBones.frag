#version 450

layout(location = 0) out vec4 out_color;

void main() {
    out_color = vec4(1, 1, 1, 1);

    for(int i = 0; i < 10000; i++) {
        out_color *= i;
    }

    out_color = max(out_color, 1);

}