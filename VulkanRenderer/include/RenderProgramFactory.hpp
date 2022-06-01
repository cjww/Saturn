#pragma once

#include "Resources\ResourceManager.hpp"

#include "Resources\Texture.hpp"

namespace sa {
	class RenderProgram;
	class Subpass;
	class VulkanCore;

	class RenderProgramFactory;

	enum SubpassAttachmentUsage {
		ColorTarget,
		DepthTarget,
		Input,
		Resolve,
	};

	class RenderProgramFactory {
	public:

		struct AttachmentReference {
			uint32_t index;
			SubpassAttachmentUsage usage;
		};

		class SubpassFactory {
		private:
			Subpass* m_pSubpass;
			RenderProgramFactory* m_pProgramFactory;
			std::vector<AttachmentReference> m_attachmentReferences;
		public:
			SubpassFactory(RenderProgramFactory* pProgramFactory, Subpass* pSubpass);

			SubpassFactory& addAttachmentReference(uint32_t index, SubpassAttachmentUsage usage);

			RenderProgramFactory& endSubpass();

			std::vector<AttachmentReference> getAttachmentReferences() const;

		};
	private:
		RenderProgram* m_pProgram;
		ResourceID m_id;
		VulkanCore* m_pCore;

		std::vector<SubpassFactory> m_subpasses;

		void onSubpassEnd(SubpassFactory subpassFactory);

	public:

		RenderProgramFactory(VulkanCore* pCore);
		
		RenderProgramFactory& addColorAttachment(bool store);
		RenderProgramFactory& addColorAttachment(bool store, const Texture2D& framebufferTexture);
		
		RenderProgramFactory& addSwapchainAttachment(ResourceID swapchain);
		RenderProgramFactory& addDepthAttachment();
		RenderProgramFactory& addDepthAttachment(const Texture2D& framebufferTexture);



		SubpassFactory beginSubpass();
		RenderProgramFactory& addSubpassDependency();
		

		ResourceID end();


	};
}