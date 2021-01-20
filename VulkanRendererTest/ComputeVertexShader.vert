#version 450

layout(location = 0) in vec4 in_position;

layout(set = 0, binding = 0) uniform UBO {
	mat4 view;
	mat4 projection;
	int matrixCount;
}ubo;

layout(set = 0, binding = 1) buffer MatrixBuffer {
	mat4 model[];
} matBuffer;

void main() {

	gl_Position = ubo.projection * ubo.view * matBuffer.model[gl_InstanceIndex] * in_position;

}