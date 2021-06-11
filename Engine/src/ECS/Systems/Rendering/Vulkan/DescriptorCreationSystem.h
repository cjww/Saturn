#pragma once
#include "ECS/System.h"
#include "ECS/ECSCoordinator.h"
#include "ECS/Components.h"
#include <unordered_map>

#include "structs.h"

namespace vk {
	class DescriptorCreationSystem : public System {
	private:

		std::queue<EntityID> m_uninitializedEntities;
		/*
		*/
		std::unordered_map<EntityID, vr::DescriptorSetPtr> m_descriptorSets;
		std::unordered_map<EntityID, std::unordered_map<uint32_t, vr::Buffer*>> m_buffers;


		vr::ShaderSet* m_pShaderSet;

		friend class SystemFactory;
		virtual bool removeEntity(EntityID entity) override;
		virtual bool addEntity(EntityID entity) override;
	public:
		DescriptorCreationSystem(vr::ShaderSet* pShaderSet);
		virtual void update(float dt);

		vr::DescriptorSetPtr getDescriptorSet(EntityID entity) const;
		vr::Buffer* getBuffer(EntityID entity, uint32_t binding) const;
	};
}