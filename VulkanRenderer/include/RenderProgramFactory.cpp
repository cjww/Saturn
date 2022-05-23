#include "pch.h"
#include "RenderProgramFactory.hpp"

#include "Resources/RenderProgram.hpp"

sa::RenderProgramFactory::RenderProgramFactory(VulkanCore* pCore) {
	m_pCore = pCore;
	m_id = sa::ResourceManager::get().insert<RenderProgram>();
	m_pProgram = sa::ResourceManager::get().get<RenderProgram>(m_id);
}

ResourceID sa::RenderProgramFactory::end() {
	m_pProgram->create(m_pCore);

	return m_id;
}
