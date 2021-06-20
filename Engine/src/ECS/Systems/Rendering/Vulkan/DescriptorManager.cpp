#include "DescriptorManager.h"

namespace vk {
	DescriptorManager::DescriptorManager(vr::ShaderSet* pShaderSet)
		: m_pShaderSet(pShaderSet)
	{
	}

	void DescriptorManager::addEntityDescriptorSet(EntityID entity) {
		vr::DescriptorSetPtr descSet = m_pShaderSet->getDescriptorSet(SET_PER_OBJECT);
	
		PerObjectBuffer initialData = {};
		initialData.worldMatrix = glm::mat4(1);
		vr::Buffer* buffer = vr::Renderer::get()->createUniformBuffer(sizeof(PerObjectBuffer), &initialData);

		vr::Renderer::get()->updateDescriptorSet(descSet, 0, buffer, nullptr, nullptr, true);

		m_buffers.insert(std::make_pair(entity, buffer));
		m_descriptorSets.insert(std::make_pair(entity, descSet));
	}

	void DescriptorManager::removeEntityDescriptorset(EntityID entity) {
		m_descriptorSets.erase(entity);
		m_buffers.erase(entity);
	}

	vr::DescriptorSetPtr DescriptorManager::getDescriptorSet(EntityID entity) const {
		return m_descriptorSets.at(entity);
	}

	vr::Buffer* DescriptorManager::getBuffer(EntityID entity) const {
		return m_buffers.at(entity);
	}
}