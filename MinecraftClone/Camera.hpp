#pragma once

#include <Renderer.hpp>
#include "CameraController.hpp"

class Camera {
private:
	struct BufferObject {
		glm::mat4 view;
		glm::mat4 projection;
	};

	vr::BufferPtr m_buffer;

	vr::DescriptorSetPtr m_descriptorSet;
	uint32_t m_binding;

	CameraController m_controller;

	BufferObject m_ubo;

public:
	Camera(vr::RenderWindow& window, vr::DescriptorSetPtr viewProjectionSet, uint32_t binding);
	~Camera();

	void update(float dt);
};

