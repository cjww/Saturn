#pragma once
#include "Renderer.hpp"
#include "ECS\EntityFactory.h"

#include "structs.h"

namespace vk {
	class DescriptorManager {
	private:
		vr::ShaderSet* m_pShaderSet;

		std::unordered_map<EntityID, vr::DescriptorSetPtr> m_descriptorSets;
		std::unordered_map<EntityID, vr::Buffer*> m_buffers;

	public:
		DescriptorManager(vr::ShaderSet* pShaderSet);
		DescriptorManager(const DescriptorManager&) = delete;
		void operator=(const DescriptorManager&) = delete;
		virtual ~DescriptorManager() = default;
	
		void addEntityDescriptorSet(EntityID entity);
		void removeEntityDescriptorset(EntityID entity);

		vr::DescriptorSetPtr getDescriptorSet(EntityID entity) const;
		vr::Buffer* getBuffer(EntityID entity) const;
	};
}