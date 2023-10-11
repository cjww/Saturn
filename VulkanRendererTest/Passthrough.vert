#version 450

layout (location = 0) in vec4 in_vertexPosition;
layout (location = 1) in vec4 in_vertexColor;
layout (location = 2) in vec2 in_vertexUV;

layout (location = 0) out vec2 out_vertexUV;
layout (location = 1) out vec4 out_vertexColor;

struct Material {
    vec4 color;
    float time;
};

layout (set = 0, binding = 0) uniform Scene {
    mat4 view;
    mat4 projection;
} scene;

layout (set = 1, binding = 0) uniform Object {
    mat4 model;
    Material material;
} object;

layout(push_constant) uniform PC {
    float timer;
} pc;

void main() {
    mat4 vp = scene.projection * scene.view;
	
    vec4 vertexWorldPos = object.model * in_vertexPosition;

    vec4 color = object.material.color * in_vertexColor;

    vertexWorldPos.x += sin(pc.timer * object.material.time);

    out_vertexColor = color;

    out_vertexUV = in_vertexUV;
    gl_Position = vp * vertexWorldPos;
    
}