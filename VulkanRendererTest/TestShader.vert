#version 450


const vec3 triangle[3] = vec3[3](
    vec3(0.5, 0.5, 0.0),
    vec3(0.0, -0.5, 0.0),
    vec3(-0.5, 0.5, 0.0)
);

void main() {

    gl_Position = vec4(triangle[gl_VertexIndex], 1.0);
}