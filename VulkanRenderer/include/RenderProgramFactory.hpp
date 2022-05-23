#pragma once

#include "Resources\ResourceManager.hpp"

namespace sa {
	class RenderProgram;
	class VulkanCore;

	class RenderProgramFactory {
	private:
		RenderProgram* m_pProgram;
		ResourceID m_id;
		VulkanCore* m_pCore;
	public:
		RenderProgramFactory(VulkanCore* pCore);
		
		RenderProgramFactory& addAttachment();


		RenderProgramFactory& beginSubpass();
		RenderProgramFactory& pushAttachmentRef();
		RenderProgramFactory& endSubpass();


		ResourceID end();


	};
}