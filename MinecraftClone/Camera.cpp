#include "Camera.hpp"

Camera::Camera(vr::RenderWindow& window, vr::DescriptorSetPtr viewProjectionSet, uint32_t binding) : m_controller(window), m_descriptorSet(viewProjectionSet), m_binding(binding) {
	auto renderer = vr::Renderer::get();
	
	m_ubo.projection = m_controller.getProjection(60);
	m_ubo.view = m_controller.getView(0);
	m_buffer = renderer->createUniformBuffer(sizeof(m_ubo), &m_ubo);

	renderer->updateDescriptorSet(m_descriptorSet, binding, m_buffer, nullptr, nullptr, true);

}

Camera::~Camera() {

}

void Camera::update(float dt) {
	m_ubo.view = m_controller.getView(dt);
	memcpy((char*)m_buffer->mappedData + offsetof(BufferObject, view), &m_ubo.view, sizeof(m_ubo.view));

	vr::Renderer::get()->updateDescriptorSet(m_descriptorSet, m_binding, m_buffer, nullptr, nullptr, false);
}
