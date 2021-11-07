#version 450

vec2[4] vertices = {
    vec2(-1, 1),
    vec2(1, 1),
    vec2(-1, -1),
    vec2(1, -1)
};

int[6] indices = {
    0,
    1,
    2,
    1,
    3,
    2
};

vec2[4] uvs = {
    vec2(0, 1),
    vec2(1, 1),
    vec2(0, 0),
    vec2(1, 0)
};


layout(location = 0) out vec2 out_vertexUV;
layout(location = 1) out uint out_vertexIndex;

void main() {

    gl_Position = vec4(vertices[indices[gl_VertexIndex]], 0, 1);
    out_vertexUV = uvs[indices[gl_VertexIndex]];
    out_vertexIndex = gl_VertexIndex;
}